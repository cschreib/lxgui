#include <lxgui/xml_document.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    xml::document doc("test.xml", "test.def");
    if (!doc.check())
    {
        std::cout << "## Fail !" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "## Success !" << std::endl;
    }

    xml::block* pMain = doc.get_main_block();

    std::cout << "Num sub : " << pMain->get_child_number() << std::endl;

    for (xml::block* pSub : pMain->blocks())
    {
        std::cout << "Sub : " << pSub->get_name() << std::endl;
        std::cout << " attr1 : " << pSub->get_attribute("attr1") << std::endl;
        std::cout << " attr2 : " << pSub->get_attribute("attr2") << std::endl;
        std::cout << " attr3 : " << pSub->get_attribute("attr3") << std::endl;
        std::cout << " attr4 : " << pSub->get_attribute("attr4") << std::endl;

        for (xml::block* pSubb : pSub->blocks())
        {
            xml::block* pSub1 = pSubb->get_block("SubbTag1");
            if (pSub1)
                std::cout << "Subb1 : x " << pSub1->get_attribute("x") << std::endl;
            else
                std::cout << "# No Subb1 : boo..." << std::endl;
        }
    }

    return 0;
}
