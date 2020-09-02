#include "localscope.h"
#include "modcontext.h"
#include "module.h"
#include "ModuleInstantiation.h"
#include "node.h"
#include "UserModule.h"
#include "expression.h"
#include "function.h"
#include "annotation.h"
#include "UserModule.h"
#include <iomanip>
#include <sstream>

void LocalScope::addModuleInst(const shared_ptr<ModuleInstantiation>& modinst)
{
	assert(modinst);
	this->moduleInstantiations.push_back(modinst);
}

void LocalScope::addModule(const shared_ptr<class UserModule>& module)
{
	assert(module);
	auto it=this->modules.find(module->name);
	if(it!=this->modules.end()) it->second=module;
	else this->modules.emplace(module->name, module);
	this->astModules.emplace_back(module->name, module);
}

void LocalScope::addFunction(const shared_ptr<class UserFunction>& func)
{
	assert(func);
	auto it=this->functions.find(func->name);
	if(it!=this->functions.end()) it->second=func;
	else this->functions.emplace(func->name, func);
	this->astFunctions.emplace_back(func->name, func);
}

void LocalScope::addAssignment(const shared_ptr<Assignment>& ass)
{
	this->assignments.push_back(ass);
}

void LocalScope::print(std::ostream &stream, const std::string &indent, const bool inlined) const
{
  std::string curGroup;

  // Preserve the customizer variables at the beginning
  for (const auto &ass : this->assignments) {
    if (!ass->hasAnnotations())
      continue;

    // Add the group header if it's different from the current one.
		const Annotation *group = ass->annotation("Group");
		if (group) {
      std::string str;
      std::stringstream(group->dump()) >> std::quoted(str);
      if (!str.empty() && curGroup != str) {
        curGroup = str;
        stream << "/* [" << str << "] */" << std::endl;
      }
    }

    // Add a comment with the description before the assignment
		const Annotation *description = ass->annotation("Description");
		if (description) {
			std::string str;
			std::stringstream(description->dump()) >> std::quoted(str);
      if (!str.empty()) {
        stream << "// " << str << std::endl;
      }
    }

    // Print the assignment
    ass->print(stream, indent);

    // Print parameters on the same line
		const Annotation *parameter = ass->annotation("Parameter");
		if (parameter) {
      std::string str = parameter->dump();
      if (!str.empty() && str != "\"\"") {
        // Assignment::print ends with a newline, so we seek back -1 to remove
        // it.
        stream.seekp(-1, std::ios_base::cur);
        stream << " // " << str << std::endl;
			}
    }
  }

	for (const auto &f : this->astFunctions) {
		f.second->print(stream, indent);
	}
	for (const auto &m : this->astModules) {
		m.second->print(stream, indent);
	}
	for (const auto &ass : this->assignments) {
    if (ass->hasAnnotations())
      continue;
		ass->print(stream, indent);
	}
	for (const auto &inst : this->moduleInstantiations) {
		inst->print(stream, indent, inlined);
	}
}

AbstractNode* LocalScope::instantiateModules(const std::shared_ptr<const Context> &context, AbstractNode* target) const
{
	for(const auto &modinst : this->moduleInstantiations) {
		AbstractNode *node = modinst->evaluate(context);
		if (node) {
			target->children.push_back(node);
		}
	}
	return target;
}

AbstractNode* LocalScope::instantiateModules(const std::shared_ptr<const Context>& context, AbstractNode* target, const std::vector<size_t>& indices) const
{
	for (size_t index : indices) {
		assert(index < this->moduleInstantiations.size());
		AbstractNode *node = moduleInstantiations[index]->evaluate(context);
		if (node) {
			target->children.push_back(node);
		}
	}
	return target;
}
