#include "lxgui/xml_document.hpp"
#include "lxgui/xml_block.hpp"

#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <fstream>

namespace xml
{
document::document(const std::string& sDefFileName, std::ostream& mOut) :
    out(mOut.rdbuf()), sDefFileName_(sDefFileName), uiCurrentLineNbr_(0),
    bValid_(true), pState_(nullptr), bSmartComment_(false),
    uiSmartCommentCount_(0), bMultilineComment_(false),
    bPreProcessor_(false), uiPreProcessorCount_(0), uiSkippedPreProcessorCount_(0)
{
    mXMLState_.set_document(this);
    mDefState_.set_document(this);

    if (utils::file_exists(sDefFileName_))
    {
        mMainBlock_.set_document(this);
        bValid_ = true;

        try { load_definition_(); }
        catch (const utils::exception& e)
        {
            out << e.get_description() << std::endl;
            bValid_ = false;
        }
    }
    else
    {
        out << "# Error # : xml::document : Can't find \"" << sDefFileName_ << "\"." << std::endl;
        bValid_ = false;
    }
}

document::document(const std::string& sFileName, const std::string& sDefFileName, std::ostream& mOut) :
    out(mOut.rdbuf()),sFileName_(sFileName), sDefFileName_(sDefFileName), uiCurrentLineNbr_(0),
    bValid_(true), pState_(nullptr), bSmartComment_(false), uiSmartCommentCount_(0),
    bMultilineComment_(false), bPreProcessor_(false),
    uiPreProcessorCount_(0), uiSkippedPreProcessorCount_(0)
{
    mXMLState_.set_document(this);
    mDefState_.set_document(this);
    if (utils::file_exists(sFileName_))
    {
        if (utils::file_exists(sDefFileName_))
        {
            mMainBlock_.set_document(this);
            bValid_ = true;

            try { load_definition_(); }
            catch (const utils::exception& e)
            {
                out << e.get_description() << std::endl;
                bValid_ = false;
            }
        }
        else
        {
            out << "# Error # : xml::document : Can't find \"" << sDefFileName_ << "\"." << std::endl;
            bValid_ = false;
        }
    }
    else
    {
        out << "# Error # : xml::document : Can't find \"" << sFileName_ << "\"." << std::endl;
        bValid_ = false;
    }
}

document::~document()
{
}

void document::set_file_name(const std::string& sFileName)
{
    if (utils::file_exists(sFileName))
        sFileName_ = sFileName;
    else
    {
        out << "# Error # : xml::document : Can't find \"" << sFileName << "\"." << std::endl;
        bValid_ = false;
    }
}

void document::set_source_string(const std::string& sString)
{
    sSourceString_ = sString;
}

block* document::get_main_block()
{
    return &mMainBlock_;
}

const std::string& document::get_current_file_name() const
{
    return sCurrentFileName_;
}

uint document::get_current_line_nbr() const
{
    return uiCurrentLineNbr_;
}

std::string document::get_current_location() const
{
    return sCurrentFileName_+":"+utils::to_string(uiCurrentLineNbr_);
}

void document::load_definition_()
{
    // Open the def file
    std::ifstream mFile(sDefFileName_);
    sCurrentFileName_ = sDefFileName_;
    uiCurrentLineNbr_ = 0u;
    std::string sLine, sTemp;
    pState_ = &mDefState_;

    // Start parsing line by line
    while (!mFile.eof() && bValid_)
    {
        getline(mFile, sTemp);
        utils::replace(sTemp, "\r", "");
        sLine += sTemp;
        ++uiCurrentLineNbr_;

        if (!utils::has_no_content(sLine))
        {
            // Read all tags on this line
            read_tags_(sLine);
        }
    }
}

std::vector<std::string> read_preprocessor_commands(const std::string& sCommands)
{
    std::vector<std::string> lPreProcessorCommands = utils::cut(sCommands, ",");
    for (auto& sCommand : lPreProcessorCommands)
        utils::trim(sCommand, ' ');

    return lPreProcessorCommands;
}

void document::read_opening_tag_(std::string& sTagContent)
{
    if (sTagContent[0] == '#') // Preprocessor
    {
        ++uiPreProcessorCount_;
        if (!bPreProcessor_)
        {
            // Extract the required identifiers
            size_t uiStart = sTagContent.find("[");
            size_t uiEnd   = sTagContent.find("]");
            if (uiStart != sTagContent.npos && uiEnd != sTagContent.npos && (uiStart < uiEnd) && (uiEnd-uiStart > 1))
            {
                std::vector<std::string> lIDList = utils::cut(sTagContent.substr(uiStart+1, uiEnd-(uiStart+1)), ",");
                std::vector<std::string>::iterator iterID;
                for (auto& sID : lIDList)
                {
                    utils::trim(sID, ' ');
                    std::string sCommand = sID;
                    if (!sCommand.empty() && sCommand[0] == '!')
                        sCommand.erase(0, 1);

                    // Compare with the preprocessor commands given by the user
                    bool b = utils::find(lPreProcessorCommands_, sCommand) != lPreProcessorCommands_.end();
                    if (b == (sID[0] == '!'))
                    {
                        // Skip all the following code until the proper ending tag is found
                        bPreProcessor_ = true;
                        uiSkippedPreProcessorCount_ = 1u;
                    }
                }
            }
            else
            {
                out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
                    << " : PreProcessor command with no argument is always 'true' "
                    << "(expected \"<#[some parameter]>\")." << std::endl;
            }
        }
        else
            ++uiSkippedPreProcessorCount_;
    }
    else if (!bPreProcessor_)
    {
        if (sTagContent[0] == '!') // Smart comment
        {
            if (!bMultilineComment_ && !bSmartComment_)
            {
                bSmartComment_ = true;
                // Remove the exclamation mark
                sTagContent.erase(0, 1);
                // Extract the commented tag's name
                sSmartCommentTag_ = utils::cut(sTagContent, " ", 1).front();
                uiSmartCommentCount_ = 1u;
            }
        }
        else // Regular opening tag
        {
            std::string sName = pState_->read_tag_name(sTagContent);
            if (bSmartComment_)
            {
                if (sName == sSmartCommentTag_)
                    ++uiSmartCommentCount_;
            }
            else
            {
                try { pState_->read_opening_tag(sTagContent); }
                catch (const utils::exception& e)
                {
                    if (pState_->get_id() == state::STATE_DEF)
                    {
                        // The definition stage throws exceptions
                        // that are catched in the constructor.
                        throw;
                    }
                    else
                    {
                        // The loading stage just prints things in
                        // the log file.
                        out << "# Error # : " << e.get_description() << std::endl;
                    }
                }
            }
        }
    }
}

void document::read_single_tag_(std::string& sTagContent)
{
    // Remove the trailling slash
    sTagContent.erase(sTagContent.length()-1, 1);

    if (sTagContent[0] == '#') // PreProcessor
    {
        out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
            << " : Single block PreProcessor tag is useless. Skipped." << std::endl;
    }
    else if (!bPreProcessor_)
    {
        if (sTagContent[0] == '!') // Smart comment
        {
            // Smart comment on a single tag, just ignore the tag
        }
        else if (!bSmartComment_) // Regular single tag
        {
            try
            {
                pState_->read_single_tag(sTagContent);
            }
            catch (const utils::exception& e)
            {
                if (pState_->get_id() == state::STATE_DEF)
                {
                    // The definition stage throws exceptions
                    // that are catched in the constructor.
                    throw;
                }
                else
                {
                    // The loading stage just prints things in
                    // the log file.
                    out << "# Error # : " << e.get_description() << std::endl;
                }
            }
        }
    }
}

void document::read_ending_tag_(std::string& sTagContent)
{
    // Remove the starting slash
    sTagContent.erase(0, 1);

    if (sTagContent[0] == '#') // PreProcessor
    {
        if (uiPreProcessorCount_ != 0)
        {
            --uiPreProcessorCount_;
            if (bPreProcessor_)
            {
                --uiSkippedPreProcessorCount_;
                if (uiSkippedPreProcessorCount_ == 0)
                {
                    // End of the skiped code
                    bPreProcessor_ = false;
                }
            }
        }
        else
        {
            out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
                << " : PreProcessor end tag in excess. Skipped." << std::endl;
        }
    }
    else if (!bPreProcessor_) // Regular end tag
    {
        std::string sName = pState_->read_tag_name(sTagContent);
        if (bSmartComment_)
        {
            if (sName == sSmartCommentTag_)
            {
                // It has the same name as the current "smart
                // commented" block
                --uiSmartCommentCount_;
                if (uiSmartCommentCount_ == 0)
                    bSmartComment_ = false;
            }
        }
        else
        {
            try
            {
                pState_->read_ending_tag(sTagContent);
            }
            catch (const utils::exception& e)
            {
                if (pState_->get_id() == state::STATE_DEF)
                {
                    // The definition stage throws exceptions
                    // that are catched in the constructor.
                    throw;
                }
                else
                {
                    // The loading stage just prints things in
                    // the log file.
                    out << "# Error # : " << e.get_description() << std::endl;
                }
            }
        }
    }
}

void document::read_tags_(std::string& sLine)
{
    const char* sCommentStart        = "<!--";
    const uint  uiCommentStartLength = 4;
    const char* sCommentEnd          = "-->";
    const uint  uiCommentEndLength   = 3;

    if (!bMultilineComment_)
    {
        // Search for multiline comments
        size_t uiFirst  = sLine.find(sCommentStart);
        size_t uiSecond = sLine.find(sCommentEnd);
        while (uiSecond < uiFirst)
        {
            // Ignore misplaced comment end token
            out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
                << " : Multiline comment end token in excess (\"" << sCommentEnd << "\"). Ignored." << std::endl;

            sLine.erase(uiSecond, uiCommentEndLength);
            uiFirst  = sLine.find(sCommentStart);
            uiSecond = sLine.find(sCommentEnd);
        }

        while (uiFirst != sLine.npos)
        {
            // Multiline comment started
            uiSecond = sLine.find(sCommentEnd, uiFirst);
            if (uiSecond != sLine.npos)
            {
                // It ends on the same line
                sLine.erase(uiFirst, uiSecond+uiCommentEndLength - uiFirst);
            }
            else
            {
                // It ends later (hopefuly)
                bMultilineComment_ = true;
                break;
            }

            uiFirst = sLine.find(sCommentStart);
        }

        if (!bMultilineComment_)
        {
            // Check there are no misplaced comment end token
            if (sLine.find(sCommentEnd) != sLine.npos)
            {
                out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
                    << " : Multiline comment end token in excess (\"" << sCommentEnd << "\"). Ignored." << std::endl;
                utils::replace(sLine, sCommentEnd, "");
            }

            // Parse the tag as usual
            uiFirst = sLine.find("<");
            uiSecond = sLine.find(">", uiFirst);
            while (uiFirst != sLine.npos && uiSecond != sLine.npos && bValid_)
            {
                pState_->add_content(sLine.substr(0, uiFirst));
                std::string sTagContent = sLine.substr(uiFirst+1, uiSecond - (uiFirst+1));

                if (sTagContent[0] == '/')
                {
                    // Ending tag
                    read_ending_tag_(sTagContent);
                }
                else if (sTagContent[sTagContent.length()-1] == '/')
                {
                    // Single tag
                    read_single_tag_(sTagContent);
                }
                else
                {
                    // Opening tag
                    read_opening_tag_(sTagContent);
                }

                sLine.erase(0, uiSecond+1);
                uiFirst = sLine.find("<");
                uiSecond = sLine.find(">", uiFirst);
            }
        }
    }
    else
    {
        size_t uiOpenCount = 0;
        size_t uiLineSize = sLine.size();
        size_t uiPos = 0;
        size_t uiStart = sLine.npos;
        while (uiPos < uiLineSize)
        {
            if (uiPos+uiCommentStartLength < uiLineSize && sLine.substr(uiPos, uiCommentStartLength) == sCommentStart)
            {
                if (uiOpenCount == 0)
                    uiStart = uiPos;

                ++uiOpenCount;
                uiPos += uiCommentStartLength;

                continue;
            }
            else if (uiPos+uiCommentEndLength < uiLineSize && sLine.substr(uiPos, uiCommentEndLength) == sCommentEnd)
            {
                if (uiOpenCount == 0)
                {
                    out << "# Warning # : " << sCurrentFileName_ << ":" << uiCurrentLineNbr_
                        << " : Multiline comment end token in excess (\"" << sCommentEnd << "\"). Ignored." << std::endl;
                    sLine.erase(uiPos, uiCommentEndLength);
                }
                else
                {
                    --uiOpenCount;

                    if (uiOpenCount == 0)
                    {
                        std::size_t uiNumChar = uiPos + uiCommentEndLength - uiStart;
                        sLine.erase(uiStart, uiNumChar);
                        uiLineSize -= uiNumChar;
                        uiPos -= uiNumChar;
                        uiStart = sLine.npos;
                    }

                    uiPos += uiCommentEndLength;
                }

                continue;
            }

            ++uiPos;
        }

        if (uiOpenCount == 0)
        {
            bMultilineComment_ = false;
            read_tags_(sLine);
        }
    }
}

bool document::check(const std::string& sPreProcCommands)
{
    if (!bValid_)
        return false;

    // Parse preprocessor commands given by the user
    lPreProcessorCommands_ = read_preprocessor_commands(sPreProcCommands);

    if (!sFileName_.empty())
    {
        // Open the XML file
        std::ifstream mFile(sFileName_);
        sCurrentFileName_ = sFileName_;
        uiCurrentLineNbr_ = 0u;
        pState_ = &mXMLState_;
        pState_->set_current_block(&mMainBlock_);

        // Start parsing line by line
        std::string sLine, sTemp;
        while (!mFile.eof() && bValid_)
        {
            getline(mFile, sTemp);
            utils::replace(sTemp, "\r", "");
            sLine += sTemp + "\n";
            ++uiCurrentLineNbr_;

            if (!utils::has_no_content(sLine))
            {
                // Read all tags on this line
                read_tags_(sLine);
            }
        }
    }
    else
    {
        // Read the source string
        sCurrentFileName_ = "source string";
        uiCurrentLineNbr_ = 0u;
        std::string sLine;
        pState_ = &mXMLState_;
        pState_->set_current_block(&mMainBlock_);

        std::vector<std::string> lLines = utils::cut_each(sSourceString_, "\n");
        std::vector<std::string>::iterator iterLine;

        // Start parsing line by line
        for (const auto& sTempLine : lLines)
        {
            sLine += sTempLine + "\n";
            ++uiCurrentLineNbr_;

            if (!utils::has_no_content(sLine))
            {
                // Read all tags on this line
                read_tags_(sLine);
            }

            if (!bValid_)
                break;
        }
    }

    return bValid_;
}

void document::create_predefined_block(const std::string& sName, const std::string& sInheritance)
{
    if (sInheritance.empty())
        lPredefinedBlockList_[sName] = block();
    else
        lPredefinedBlockList_[sName] = lPredefinedBlockList_[sInheritance];
}

const block* document::get_predefined_block(const std::string& sName) const
{
    std::map<std::string, block>::const_iterator iter = lPredefinedBlockList_.find(sName);
    if (iter != lPredefinedBlockList_.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }
}

block* document::get_predefined_block(const std::string& sName)
{
    std::map<std::string, block>::iterator iter = lPredefinedBlockList_.find(sName);
    if (iter != lPredefinedBlockList_.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }
}

void document::set_invalid()
{
    bValid_ = false;
}

document::state::state() : pDoc_(nullptr), pCurrentBlock_(nullptr), pCurrentParentBlock_(nullptr)
{
}

document::state::~state()
{
}

void document::state::set_document(document* pDoc)
{
    pDoc_ = pDoc;
}

void document::state::set_current_block(block* pBlock)
{
    pCurrentBlock_ = pBlock;
}

void document::state::set_current_parent_block(block* pParentBlock)
{
    pCurrentParentBlock_ = pParentBlock;
}

void document::state::add_content(const std::string& sContent)
{
    if (pCurrentBlock_)
        pCurrentBlock_->add_value(sContent);
}

const document::state::id& document::state::get_id() const
{
    return mID_;
}

document::xml_state::xml_state() : state()
{
    mID_ = STATE_XML;
}

std::string document::xml_state::read_tag_name(const std::string& sTagContent) const
{
    return utils::cut(sTagContent, " ", 1).front();
}

void document::xml_state::read_single_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    std::string sAttributes;
    if (lWords.size() > 1)
    {
        sAttributes = lWords.back();
        utils::replace(sAttributes, " =", "=");
        utils::replace(sAttributes, "= ", "=");
    }

    if (pCurrentParentBlock_)
    {
        if (pCurrentParentBlock_->can_have_block(sName))
        {
            pCurrentBlock_ = pCurrentParentBlock_->create_block(sName);
            if (!pCurrentBlock_->check_attributes(sAttributes))
                pDoc_->set_invalid();
            if (!pCurrentBlock_->check_blocks())
                pDoc_->set_invalid();

            pCurrentBlock_ = pCurrentParentBlock_;
            pCurrentParentBlock_->add_block();
        }
        else
        {
            pDoc_->set_invalid();
            throw utils::exception(pDoc_->get_current_location(), "Unexpected content : \"<"+sName+">\".");
        }
    }
    else
    {
        // This is the main block
        if (sName == pDoc_->get_main_block()->get_name())
        {
            if (!pDoc_->get_main_block()->check_attributes(sAttributes))
                pDoc_->set_invalid();
            if (!pDoc_->get_main_block()->check_blocks())
                pDoc_->set_invalid();
        }
        else
        {
            pDoc_->set_invalid();
            throw utils::exception(pDoc_->get_current_location(),
                "Wrong main block : \"<"+sName+">\". Expected : \"<"+pDoc_->get_main_block()->get_name()+">\"."
            );
        }
    }
}

void document::xml_state::read_ending_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    if (lWords.size() > 1)
    {
        pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
            << " : An end tag should only contain the closed block's name." << std::endl;
    }

    if (sName == pCurrentBlock_->get_name())
    {
        // This is the expected end tag
        pCurrentParentBlock_ = pCurrentBlock_->get_parent();
        if (pCurrentParentBlock_)
        {
            // It's a regular block's end tag
            if (!pCurrentBlock_->check_blocks())
                pDoc_->set_invalid();

            pCurrentParentBlock_->add_block();
            pCurrentBlock_ = pCurrentParentBlock_;
        }
        else
        {
            // It's the main block's end tag
            if (!pCurrentBlock_->check_blocks())
                pDoc_->set_invalid();
        }
    }
    else
    {
        pDoc_->set_invalid();
        throw utils::exception(pDoc_->get_current_location(),
            "Wrong end tag : \"</"+sName+">\". Expected : \"</"+
            pCurrentBlock_->get_name()+">\"."
        );
    }
}

void document::xml_state::read_opening_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    std::string sAttributes;
    if (lWords.size() > 1)
    {
        sAttributes = lWords.back();
        utils::replace(sAttributes, " =", "=");
        utils::replace(sAttributes, "= ", "=");
    }

    if (pCurrentParentBlock_)
    {
        if (pCurrentParentBlock_->can_have_block(sName))
        {
            pCurrentBlock_ = pCurrentParentBlock_->create_block(sName);
            if (!pCurrentBlock_->check_attributes(sAttributes))
                pDoc_->set_invalid();
            pCurrentParentBlock_ = pCurrentBlock_;
        }
        else
        {
            pDoc_->set_invalid();
            throw utils::exception(pDoc_->get_current_location(), "Unexpected content : \"<"+sName+">\".");
        }
    }
    else
    {
        // This is the main block
        if (sName == pDoc_->get_main_block()->get_name())
        {
            if (!pDoc_->get_main_block()->check_attributes(sAttributes))
                pDoc_->set_invalid();
            pCurrentParentBlock_ = pDoc_->get_main_block();
        }
        else
        {
            pDoc_->set_invalid();
            throw utils::exception(pDoc_->get_current_location(),
                "Wrong main block : \"<"+sName+">\". Expected : \"<"+pDoc_->get_main_block()->get_name()+">\"."
            );
        }
    }
}

document::def_state::def_state() : state()
{
    mID_ = STATE_DEF;
}

std::string document::def_state::read_tag_name(const std::string& sTagContent) const
{
    return utils::cut(utils::cut(sTagContent, " ", 1).front(), ":").back();
}

void document::def_state::read_predef_commands_(std::string& sName, std::string& sParent, uint& uiMin, uint& uiMax, bool& bCopy, bool& bPreDefining, bool& bLoad, uint& uiRadioGroup, bool bMultiline)
{
    std::vector<std::string> lCommands = utils::cut(sName, ":");
    sName = lCommands.back();
    lCommands.pop_back();

    for (const auto& sCommand : lCommands)
    {
        char sLetterCode = sCommand[0];
        if ((sLetterCode == 'd') || (sLetterCode == 'c'))
        {
            if (sLetterCode == 'c')
                bCopy = true;

            // Pre-definintion
            if (pCurrentParentBlock_ && (sLetterCode == 'd'))
            {
                throw utils::exception(pDoc_->get_current_location(),
                    "Can't pre-define a block outside root level (nested \'d\' command forbidden)."
                );
            }
            else
            {
                bPreDefining = true;

                size_t uiStart = sCommand.find("[");
                size_t uiEnd   = sCommand.find("]");
                if (uiStart != sCommand.npos && uiEnd != sCommand.npos)
                {
                    // Inheritance
                    sParent = sCommand.substr(uiStart+1, uiEnd - (uiStart+1));
                }
            }
        }
        else if (sLetterCode == 'l')
        {
            // Load pre-definition
            if (bMultiline)
            {
                throw utils::exception(pDoc_->get_current_location(),
                    "Can't load a pre-defined block using a multiline block (\'l\' command forbidden)."
                );
            }
            else
            {
                if (!pCurrentParentBlock_)
                {
                    throw utils::exception(pDoc_->get_current_location(),
                        "Can't load a pre-defined block at root level (\'l\' command forbidden)."
                    );
                }
                else
                {
                    bLoad = true;
                }
            }
        }
        else if (sLetterCode == 'n')
        {
            // Min/max count
            size_t uiStart = sCommand.find("[");
            size_t uiEnd   = sCommand.find("]");
            if (uiStart != sCommand.npos && uiEnd != sCommand.npos)
            {
                std::string sParams = sCommand.substr(uiStart+1, uiEnd - (uiStart+1));
                if (sParams.find(",") != sParams.npos)
                {
                    std::vector<std::string> lMinMax = utils::cut(sParams, ",");
                    std::string sMin = lMinMax.front();
                    std::string sMax = lMinMax.back();
                    if (sMin == "*")
                    {
                        uiRadioGroup = utils::string_to_uint(sMax);
                    }
                    else
                    {
                        if (sMin != ".")
                            uiMin = utils::string_to_uint(sMin);
                        if (sMax != ".")
                            uiMax = utils::string_to_uint(sMax);
                    }
                }
                else
                {
                    if (sParams == "*")
                    {
                        uiRadioGroup = 0;
                    }
                    else if (utils::is_number(sParams))
                    {
                        uiMax = uiMin = utils::string_to_uint(sParams);
                    }
                    else
                    {
                        pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
                            << " : Unknown param : \"" << sParams << "\" for \'n\' command. Skipped." << std::endl;
                    }
                }
            }
            else
            {
                pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
                    << " : \'n\' command requires some parameters. Correct synthax is : \"n[params]\". Skipped." << std::endl;
            }
        }
        else
        {
            pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
                << " : Unknown command : \'" << sCommand << "\'. Skipped." << std::endl;
        }
    }
}

void document::def_state::read_single_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    uint uiMin = 0u;
    uint uiMax = -1;
    bool bPreDefining = false;
    bool bCopy = false;
    uint uiRadioGroup = -1;
    bool bLoad = false;
    std::string sParent;

    // Read commands
    read_predef_commands_(
        sName, sParent, uiMin, uiMax, bCopy, bPreDefining, bLoad, uiRadioGroup, false
    );

    if (uiRadioGroup != (uint)(-1) && sName == ".")
    {
        pCurrentParentBlock_->set_radio_group_optional(uiRadioGroup);
        return;
    }

    // Prepare attributes
    std::vector<std::string> lAttributes;
    if (lWords.size() > 1)
    {
        std::string sAttributes = lWords.back();
        utils::replace(sAttributes, " =", "=");
        utils::replace(sAttributes, "= ", "=");

        std::string sAttr;
        bool bString = false;
        for (auto cChar : sAttributes)
        {
            if (cChar == '"')
            {
                sAttr += cChar;
                bString = !bString;
            }
            else if (cChar == ' ')
            {
                if (!bString)
                {
                    if (!sAttr.empty())
                        lAttributes.push_back(sAttr);
                    sAttr = "";
                }
                else
                    sAttr += cChar;
            }
            else
                sAttr += cChar;
        }

        if (!sAttr.empty())
            lAttributes.push_back(sAttr);
    }

    if (pCurrentParentBlock_)
    {
        if (bLoad)
        {
            if (pDoc_->get_predefined_block(sName))
            {
                if (uiRadioGroup != (uint)(-1))
                    pCurrentParentBlock_->add_predefined_radio_block(pDoc_->get_predefined_block(sName), uiRadioGroup);
                else
                    pCurrentParentBlock_->add_predefined_block(pDoc_->get_predefined_block(sName), uiMin, uiMax);

                if (lAttributes.size() != 0)
                {
                    pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
                        << " : Can't add attributes to a loaded pre-defined block." << std::endl;
                }
            }
            else
            {
                throw utils::exception(pDoc_->get_current_location(),
                    "\""+sName+"\" has not (yet?) been pre-defined."
                );
            }
        }
        else if (bCopy)
        {
            if (uiRadioGroup != (uint)(-1))
                pCurrentBlock_ = pCurrentParentBlock_->create_radio_def_block(sName, uiRadioGroup);
            else
                pCurrentBlock_ = pCurrentParentBlock_->create_def_block(sName, uiMin, uiMax);

            // Pre-definition
            if (!sParent.empty())
            {
                // Inheritance
                if (pDoc_->get_predefined_block(sParent))
                {
                    pCurrentBlock_->copy(pDoc_->get_predefined_block(sParent));
                }
                else
                {
                    pDoc_->out << "# Error # : " << pDoc_->get_current_location()
                        << "\"" << sParent << "\" has not (yet?) been pre-defined and "
                        << "cannot be copied." << std::endl;
                }
            }

            pCurrentBlock_->check_attributes_def(lAttributes);
            pCurrentBlock_ = pCurrentParentBlock_;
        }
        else
        {
            if (!pDoc_->get_predefined_block(sName))
            {
                if (uiRadioGroup != (uint)(-1))
                    pCurrentBlock_ = pCurrentParentBlock_->create_radio_def_block(sName, uiRadioGroup);
                else
                    pCurrentBlock_ = pCurrentParentBlock_->create_def_block(sName, uiMin, uiMax);

                pCurrentBlock_->check_attributes_def(lAttributes);
                pCurrentBlock_ = pCurrentParentBlock_;
            }
            else
            {
                throw utils::exception(pDoc_->get_current_location(),
                    "Defining a new block named \""+sName+"\", which is the name "
                    "of a pre-defined block (use the 'l' command to load it)."
                );
            }
        }
    }
    else
    {
        if (bPreDefining)
        {
            // Pre-definition
            if (!sParent.empty())
            {
                // Inheritance
                if (pDoc_->get_predefined_block(sParent))
                {
                    pDoc_->create_predefined_block(sName, sParent);
                    if (!bCopy)
                        pDoc_->get_predefined_block(sParent)->add_derivated(sName);
                }
                else
                {
                    throw utils::exception(pDoc_->get_current_location(),
                        "\""+sParent+"\" has not (yet?) been pre-defined and cannot be "+
                        std::string(bCopy ? "copied." : "inherited.")
                    );
                }
            }
            else
                pDoc_->create_predefined_block(sName);

            pCurrentBlock_ = pDoc_->get_predefined_block(sName);
            pCurrentBlock_->set_document(pDoc_);
            pCurrentBlock_->set_name(sName);

            pCurrentBlock_->check_attributes_def(lAttributes);
        }
        else
        {
            // Main block
            if (!pDoc_->get_predefined_block(sName))
            {
                pCurrentBlock_ = pDoc_->get_main_block();
                pCurrentBlock_->set_name(sName);

                pCurrentBlock_->check_attributes_def(lAttributes);
            }
            else
            {
                throw utils::exception(pDoc_->get_current_location(),
                    "Defining a new block named \""+sName+"\", which is the name "
                    "of a pre-defined block (use the 'l' command to load it)."
                );
            }
        }
    }
}

void document::def_state::read_ending_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    if (lWords.size() > 1)
    {
        pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
            << " : An end tag should only contain the closed block's name." << std::endl;
    }

    if (sName == pCurrentBlock_->get_name())
    {
        // It's the expected end tag
        if (!utils::has_no_content(pCurrentBlock_->get_value()))
        {
            pDoc_->out << "# Warning # : " << pDoc_->get_current_location()
                << " : In a definition file, blocks cannot contain any data. Ignored." << std::endl;
        }

        pCurrentParentBlock_ = pCurrentBlock_->get_parent();
        if (pCurrentParentBlock_)
        {
            pCurrentBlock_ = pCurrentParentBlock_;
        }
        else
        {
            // It's either the main block's end tag or a
            // pre-defined block's end tag.
            // Anyway, there is nothing to do.
        }
    }
    else
    {
        throw utils::exception(pDoc_->get_current_location(),
            "Wrong end tag : \"</"+sName+">\". Expected : \"</"+pCurrentBlock_->get_name()+">\"."
        );
    }
}

void document::def_state::read_opening_tag(const std::string& sTagContent)
{
    std::vector<std::string> lWords = utils::cut(sTagContent, " ", 1);
    std::string sName = lWords.front();

    uint uiMin = 0;
    uint uiMax = -1;
    bool bPreDefining = false;
    bool bCopy = false;
    uint uiRadioGroup = -1;
    bool bLoad = false;
    std::string sParent;

    read_predef_commands_(
        sName, sParent, uiMin, uiMax, bCopy, bPreDefining, bLoad, uiRadioGroup, true
    );

    if (uiRadioGroup != (uint)(-1) && sName == ".")
    {
        pCurrentParentBlock_->set_radio_group_optional(uiRadioGroup);
        return;
    }

    std::vector<std::string> lAttributes;
    if (lWords.size() > 1)
    {
        std::string sAttributes = lWords.back();
        utils::replace(sAttributes, " =", "=");
        utils::replace(sAttributes, "= ", "=");
        lAttributes = utils::cut(sAttributes, " ");
    }

    if (pCurrentParentBlock_)
    {
        if (bCopy)
        {
            if (uiRadioGroup != (uint)(-1))
                pCurrentBlock_ = pCurrentParentBlock_->create_radio_def_block(sName, uiRadioGroup);
            else
                pCurrentBlock_ = pCurrentParentBlock_->create_def_block(sName, uiMin, uiMax);

            // Copy
            if (!sParent.empty())
            {
                // Inheritance
                if (pDoc_->get_predefined_block(sParent))
                    pCurrentBlock_->copy(pDoc_->get_predefined_block(sParent));
                else
                {
                    throw utils::exception(pDoc_->get_current_location(),
                        "\""+sParent+"\" has not (yet?) been pre-defined and cannot be "
                        "copied."
                    );
                }
            }

            pCurrentParentBlock_ = pCurrentBlock_;
        }
        else if (!pDoc_->get_predefined_block(sName))
        {
            if (uiRadioGroup != (uint)(-1))
                pCurrentBlock_ = pCurrentParentBlock_->create_radio_def_block(sName, uiRadioGroup);
            else
                pCurrentBlock_ = pCurrentParentBlock_->create_def_block(sName, uiMin, uiMax);

            pCurrentBlock_->check_attributes_def(lAttributes);
            pCurrentParentBlock_ = pCurrentBlock_;
        }
        else
        {
            throw utils::exception(pDoc_->get_current_location(),
                "Defining a new block named \""+sName+"\", which is the name "
                "of a pre-defined block."
            );
        }
    }
    else
    {
        if (bPreDefining)
        {
            // Pre-definition
            if (!sParent.empty())
            {
                // Inheritance
                if (pDoc_->get_predefined_block(sParent))
                {
                    pDoc_->create_predefined_block(sName, sParent);
                    if (!bCopy)
                        pDoc_->get_predefined_block(sParent)->add_derivated(sName);
                }
                else
                {
                    throw utils::exception(pDoc_->get_current_location(),
                        "\""+sParent+"\" has not (yet?) been pre-defined and cannot be "+
                        std::string(bCopy ? "copied." : "inherited.")
                    );
                }
            }
            else
                pDoc_->create_predefined_block(sName);

            pCurrentBlock_ = pDoc_->get_predefined_block(sName);
            pCurrentBlock_->set_document(pDoc_);
        }
        else
        {
            // Main block
            pCurrentBlock_ = pDoc_->get_main_block();
        }

        pCurrentBlock_->set_name(sName);
        pCurrentBlock_->check_attributes_def(lAttributes);

        pCurrentParentBlock_ = pCurrentBlock_;
    }
}
}
