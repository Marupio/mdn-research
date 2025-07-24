#include "Project.h"

const std::vector<std::string> mdn::Project::m_defaultMdn2dNames(
    {"MdnA", "MdnB", "MdnC"}
);

mdn::Project::Project() {
    InsertNewMdn2d();
}