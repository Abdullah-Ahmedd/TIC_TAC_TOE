#include <iostream>
#include "tinyxml2-master/tinyxml2.h"

using namespace tinyxml2;

int main() {
    XMLDocument doc;

    // Create the XML declaration
    XMLDeclaration* decl = doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    doc.InsertFirstChild(decl);

    // Create the root element
    XMLElement* root = doc.NewElement("Users");
    doc.InsertEndChild(root);

    // Create a user element
    XMLElement* user = doc.NewElement("User");
    user->SetAttribute("id", 1);
    user->SetAttribute("name", "John Doe");
    root->InsertEndChild(user);

    // Save the XML document to a file
    doc.SaveFile("user_data.xml");

    return 0;
}
