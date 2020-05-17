#include "lxgui/xml_block.hpp"
#include "lxgui/xml_document.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_exception.hpp>

namespace lxgui {
namespace xml
{
attribute::attribute(const std::string& name, bool optional, const std::string& def, attribute_type type) :
    sName(name), sDefault(def), bOptional(optional), mType(type)
{
}

predefined_block::predefined_block(block* block, uint min, uint max, uint radio_group) :
    pBlock(block), uiMin(min), uiMax(max), uiRadioGroup(radio_group)
{
}

block::block(const std::string& sName, uint uiMinNbr, uint uiMaxNbr, const std::string& sFile,
            uint uiLineNbr, uint uiRadioGroup) :
    sName_(sName), uiMaxNumber_(uiMaxNbr), uiMinNumber_(uiMinNbr), uiRadioGroup_(uiRadioGroup),
    sFile_(sFile), uiLineNbr_(uiLineNbr)
{
}

block::~block()
{
    delete pNewBlock_;

    for (auto* pBlock : utils::range::value(lFoundBlockList_))
        delete pBlock;
}

void block::copy(block* pBlock)
{
    if (pBlock)
    {
        bRadioChilds_ = pBlock->bRadioChilds_;
        sValue_ = pBlock->sValue_;

        lRadioBlockList_ = pBlock->lRadioBlockList_;

        lAttributeList_ = pBlock->lAttributeList_;
        lDefBlockList_ = pBlock->lDefBlockList_;
        lPreDefBlockList_ = pBlock->lPreDefBlockList_;
    }
}

bool block::add(const attribute& mAttrib)
{
    lAttributeList_[mAttrib.sName] = mAttrib;
    return true;
}

void block::remove_attribute(const std::string& sAttributeName)
{
    lAttributeList_.erase(sAttributeName);
}

void block::check_attributes_def(const std::vector<std::string>& lAttribs)
{
    for (auto sAttr : lAttribs)
    {
        std::string sDefault;
        bool bOptional = false;
        if (sAttr.find("=") != sAttr.npos)
        {
            bOptional = true;
            std::vector<std::string> lCut = utils::cut(sAttr, "=");
            sAttr = lCut.front();
            sDefault = lCut.back();
            utils::trim(sDefault,'"');
        }

        attribute_type mType = attribute_type::STRING;
        std::vector<std::string> lCommands = utils::cut(sAttr, ":");
        sAttr = lCommands.back();
        lCommands.pop_back();

        bool bAdd = true;
        for (const auto& sCommand : lCommands)
        {
            char cLetterCode = sCommand[0];
            if (cLetterCode == 's')
            {
                mType = attribute_type::STRING;
            }
            else if (cLetterCode == 'n')
            {
                mType = attribute_type::NUMBER;
            }
            else if (cLetterCode == 'b')
            {
                mType = attribute_type::BOOLEAN;
            }
            else if (cLetterCode == '-')
            {
                remove_attribute(sAttr);
                bAdd = false;
            }
            else
            {
                pDoc_->out << "# Warning # : " << pDoc_->get_current_file_name() <<":"
                    << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                    << "Unknown command : \'" << sCommand << "\'. Skipped." << std::endl;
            }
        }

        if (bAdd)
            add(attribute(sAttr, bOptional, sDefault, mType));
    }
}

bool block::check_attributes(const std::string& sAttributes)
{
    if (!sAttributes.empty())
    {
        std::vector<std::string> lAttribs;
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
                        lAttribs.push_back(sAttr);
                    sAttr = "";
                }
                else
                    sAttr += cChar;
            }
            else
                sAttr += cChar;
        }

        if (!sAttr.empty())
            lAttribs.push_back(sAttr);

        for (const auto& sAttr : lAttribs)
        {
            if (sAttr.find("=") != sAttr.npos)
            {
                std::vector<std::string> lWords = utils::cut(sAttr, "=");
                std::string sAttrName = lWords.front();
                utils::trim(sAttrName, ' ');

                std::string sAttrValue = lWords.back();
                utils::trim(sAttrValue, ' ');
                utils::trim(sAttrValue, '"');

                std::map<std::string, attribute>::iterator iter = lAttributeList_.find(sAttrName);
                if (iter != lAttributeList_.end())
                {
                    attribute* pAttr = &iter->second;
                    if (pAttr->mType == attribute_type::BOOLEAN)
                    {
                        if (!utils::is_boolean(sAttrValue))
                        {
                            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                                << "Attribute \"" << sAttrName << "\" has wrong type (boolean expected)." << std::endl;
                            return false;
                        }
                    }
                    else if (pAttr->mType == attribute_type::NUMBER)
                    {
                        if (!utils::is_number(sAttrValue))
                        {
                            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                                << "Attribute \"" << sAttrName << "\" has wrong type (number expected)." << std::endl;
                            return false;
                        }
                    }

                    pAttr->bFound = true;
                    pAttr->sValue = sAttrValue;
                }
                else
                {
                    pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                        << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                        << "Unknown attribute : \"" << sAttrName << "\"." << std::endl;

                    pDoc_->out << "Listing available possibilities :" << std::endl;
                    for (const auto& sKey : utils::range::key(lAttributeList_))
                        pDoc_->out << "    " << sKey << std::endl;

                    return false;
                }
            }
            else
            {
                pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                    << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                    << "Wrong synthax for attributes (missing '=')." << std::endl;
                return false;
            }
        }
    }

    bool bGood = true;
    for (auto& mAttr : utils::range::value(lAttributeList_))
    {
        if (!mAttr.bFound)
        {
            if (mAttr.bOptional)
            {
                mAttr.sValue = mAttr.sDefault;
            }
            else
            {
                pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                    << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                    << "Missing \"" << mAttr.sName << "\" attribute." << std::endl;
                bGood = false;
            }
        }
    }

    return bGood;
}

bool block::check_blocks()
{
    if (bRadioChilds_)
    {
        for (auto& mBlock : utils::range::value(lDefBlockList_))
        {
            uint uiCount = lFoundBlockList_.count(mBlock.get_name());
            uint uiGroup = mBlock.get_radio_group();
            if (uiGroup != (uint)(-1))
            {
                if (uiCount == 1)
                {
                    if (!lRadioBlockList_[uiGroup])
                    {
                        lRadioBlockList_[uiGroup] = lFoundBlockList_.find(mBlock.get_name())->second;
                    }
                    else
                    {
                        pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                            << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                            << "\"<" << mBlock.get_name() << ">\" is part of a radio group with "
                            <<"\"<"  << lRadioBlockList_[uiGroup]->get_name()
                            << ">\", which has been found first." << std::endl;
                        return false;
                    }
                }
                else if (uiCount > 1)
                {
                    pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                        << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                        << "\"<" << mBlock.get_name() << ">\" is part of a radio group but has "
                        << "been found several times." << std::endl;
                    return false;
                }
            }
        }

        for (auto& mPreDefBlock : utils::range::value(lPreDefBlockList_))
        {
            uint uiCount = lFoundBlockList_.count(mPreDefBlock.pBlock->get_name());
            uint uiGroup = mPreDefBlock.uiRadioGroup;
            if (uiGroup != (uint)(-1))
            {
                if (uiCount == 1)
                {
                    if (!lRadioBlockList_[uiGroup])
                    {
                        lRadioBlockList_[uiGroup] = lFoundBlockList_.find(mPreDefBlock.pBlock->get_name())->second;
                    }
                    else
                    {
                        pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                            << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                            << "\"<" << mPreDefBlock.pBlock->get_name() << ">\" is part of a radio group with "
                            << "\"<" << lRadioBlockList_[uiGroup]->get_name() << ">\", which has been "
                            << "found first." << std::endl;
                        return false;
                    }
                }
                else
                {
                    pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                        << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                        << "\"<" << mPreDefBlock.pBlock->get_name() << ">\" is part of a radio group but has "
                        << "been found several times." << std::endl;
                    return false;
                }
            }
        }

        for (const auto& mPair : lRadioBlockList_)
        {
            if (!mPair.second &&
                utils::find(lOptionalRadioGroupList_, mPair.first) == lOptionalRadioGroupList_.end())
            {
                pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                    << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                    << "No block found for radio group " << mPair.first << "." << std::endl;
                return false;
            }
        }
    }

    for (const auto& mBlock : utils::range::value(lDefBlockList_))
    {
        uint uiCount = lFoundBlockList_.count(mBlock.get_name());
        if (uiCount < mBlock.get_min_count())
        {
            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                << "Too few \"<" << mBlock.get_name() << ">\" blocks (expected : at least "
                << mBlock.get_min_count() << ")." << std::endl;
            return false;
        }
        else if (uiCount > mBlock.get_max_count())
        {
            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                << "Too many \"<" << mBlock.get_name() << ">\" blocks (expected : at most "
                << mBlock.get_max_count() << ")." << std::endl;
            return false;
        }
    }

    for (const auto& mPreDefBlock : utils::range::value(lPreDefBlockList_))
    {
        uint uiCount = lFoundBlockList_.count(mPreDefBlock.pBlock->get_name());
        if (uiCount < mPreDefBlock.uiMin)
        {
            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                << "Too few \"<" << mPreDefBlock.pBlock->get_name() << ">\" blocks (expected : at least "
                << mPreDefBlock.uiMin << ")." << std::endl;
            return false;
        }
        else if (uiCount > mPreDefBlock.uiMax)
        {
            pDoc_->out << "# Error # : " << pDoc_->get_current_file_name() << ":"
                << pDoc_->get_current_line_nbr() << " : " << sName_ << " : "
                << "Too many \"<" << mPreDefBlock.pBlock->get_name() << ">\" blocks (expected : at most "
                << mPreDefBlock.uiMax << ")." << std::endl;
            return false;
        }
    }

    return true;
}

const std::string& block::get_name() const
{
    return sName_;
}

void block::set_name(const std::string& sName)
{
    sName_ = sName;
}

const std::string& block::get_value() const
{
    return sValue_;
}

void block::add_value(const std::string& sValue)
{
    sValue_ += sValue;
}

block* block::get_parent() const
{
    return pParent_;
}

void block::set_parent(block* pParent)
{
    pParent_ = pParent;
}

void block::add_derivated(const std::string& sName)
{
    lDerivatedList_.push_back(sName);
}

bool block::has_derivated(const std::string& sName) const
{
    if (utils::find(lDerivatedList_, sName) != lDerivatedList_.end())
    {
        return true;
    }
    else
    {
        for (const auto& sDerivated : lDerivatedList_)
        {
            if (pDoc_->get_predefined_block(sDerivated)->has_derivated(sName))
            {
                return true;
            }
        }
    }

    return false;
}

void block::set_document(document* pDoc)
{
    pDoc_ = pDoc;
}

uint block::get_min_count() const
{
    return uiMinNumber_;
}

uint block::get_max_count() const
{
    return uiMaxNumber_;
}

bool block::is_radio() const
{
    return uiRadioGroup_ != (uint)(-1);
}

uint block::get_radio_group() const
{
    return uiRadioGroup_;
}

bool block::has_radio_children() const
{
    return bRadioChilds_;
}

void block::set_radio_group_optional(uint uiGroup)
{
    if (utils::find(lOptionalRadioGroupList_, uiGroup)
        == lOptionalRadioGroupList_.end())
    {
        lOptionalRadioGroupList_.push_back(uiGroup);
    }
    else
    {
        pDoc_->out << "# Warning # : " << sFile_ << ":" << uiLineNbr_ << " : " << sName_ << " : "
            << "Radio group " << uiGroup << " has already been flagged as optional. Ignoring." << std::endl;
    }
}

uint block::get_def_child_number() const
{
    return lDefBlockList_.size() + lPreDefBlockList_.size();
}

uint block::get_child_number() const
{
    return lFoundBlockList_.size();
}

uint block::get_child_number(const std::string& sName) const
{
    return lFoundBlockList_.count(sName);
}

block* block::first(const std::string& sName)
{
    if (sName.empty())
    {
        if (!lFoundBlockStack_.empty())
        {
            mCurrIter_ = lFoundBlockStack_.begin();
            mEndIter_ = lFoundBlockStack_.end();
            return (*mCurrIter_)->second;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        std::map<std::string, std::vector<found_block_iterator>>::iterator iter;
        iter = lFoundBlockSortedStacks_.find(sName);
        if (iter != lFoundBlockSortedStacks_.end())
        {
            mCurrIter_ = iter->second.begin();
            mEndIter_ = iter->second.end();
            return (*mCurrIter_)->second;
        }
        else
        {
            return nullptr;
        }
    }
}

block* block::next()
{
    if (mCurrIter_ != mEndIter_)
    {
        ++mCurrIter_;
        if (mCurrIter_ == mEndIter_)
            return nullptr;
        else
            return (*mCurrIter_)->second;
    }
    else
        return nullptr;
}

std::string block::get_attribute(const std::string& sName)
{
    std::map<std::string, attribute>::iterator iter = lAttributeList_.find(sName);
    if (iter != lAttributeList_.end())
    {
        return iter->second.sValue;
    }
    else
    {
        pDoc_->out << "# Error # : " << get_location() << " : "
            << "Attribute \"" << sName << "\" for '" << sName_ << "' doesn't exist." << std::endl;

        pDoc_->out << "List :" << std::endl;
        std::map<std::string, attribute>::iterator iterAttr;
        for (const auto& sKey : utils::range::key(lAttributeList_))
            pDoc_->out << "    " << sKey << std::endl;

        return "";
    }
}

bool block::is_provided(const std::string& sName)
{
    std::map<std::string, attribute>::iterator iter = lAttributeList_.find(sName);
    if (iter != lAttributeList_.end())
        return iter->second.bFound;
    else
        return false;
}

block* block::get_block(const std::string& sName)
{
    std::multimap< std::string, block* >::iterator iter = lFoundBlockList_.find(sName);
    if (iter != lFoundBlockList_.end())
        return iter->second;
    else
        return nullptr;
}

block* block::get_radio_block(uint uiGroup)
{
    if (bRadioChilds_)
    {
        std::map<uint, block*>::iterator iter = lRadioBlockList_.find(uiGroup);
        if (iter != lRadioBlockList_.end())
            return iter->second;
        else
        {
            pDoc_->out << "# Warning # : " << sFile_ << ":" << uiLineNbr_ << ":" << sName_
                << " : No block in radio group " << uiGroup << "." << std::endl;
            return nullptr;
        }
    }
    else
    {
        pDoc_->out << "# Warning # : " << sFile_ << ":" << uiLineNbr_ << ":" << sName_
            << " : This block isn't meant to contain radio children." << std::endl;
        return nullptr;
    }
}

bool block::has_block(const std::string& sName)
{
    return (lDefBlockList_.find(sName) != lDefBlockList_.end()
        || lPreDefBlockList_.find(sName) != lPreDefBlockList_.end());
}

bool block::can_have_block(const std::string& sName)
{
    if (has_block(sName))
        return true;

    for (const auto& sPreDefName : utils::range::key(lPreDefBlockList_))
    {
        const block* pGlobal = pDoc_->get_predefined_block(sPreDefName);
        if (pGlobal->has_derivated(sName))
        {
            return true;
        }
    }

    return false;
}

block* block::create_block(const std::string& sName)
{
    if (!bCreating_)
    {
        std::map<std::string, block>::iterator iter = lDefBlockList_.find(sName);
        if (iter != lDefBlockList_.end())
            pNewBlock_ = new block(iter->second);
        else
            pNewBlock_ = new block(*pDoc_->get_predefined_block(sName));

        pNewBlock_->set_file(pDoc_->get_current_file_name());
        pNewBlock_->set_line_nbr(pDoc_->get_current_line_nbr());
        pNewBlock_->set_parent(this);
        pNewBlock_->set_document(pDoc_);
        bCreating_ = true;
        return pNewBlock_;
    }
    else
    {
        pDoc_->out << "# Error # : xml::block : Already creating a block." << std::endl;
        return nullptr;
    }
}

void block::add_block()
{
    if (bCreating_)
    {
        found_block_iterator iterAdded;
        // Store the new block
        iterAdded = lFoundBlockList_.insert(std::make_pair(pNewBlock_->get_name(), pNewBlock_));
        // Position it on the global stack
        lFoundBlockStack_.push_back(iterAdded);
        // Position it on the sorted stack
        lFoundBlockSortedStacks_[pNewBlock_->get_name()].push_back(iterAdded);

        pNewBlock_ = nullptr;
        bCreating_ = false;
    }
}

block* block::create_def_block(const std::string& sName, uint uiMinNbr, uint uiMaxNbr)
{
    lDefBlockList_[sName] = block(sName, uiMinNbr, uiMaxNbr, pDoc_->get_current_file_name(), pDoc_->get_current_line_nbr());
    block* pBlock = &lDefBlockList_[sName];
    pBlock->set_parent(this);
    pBlock->set_document(pDoc_);
    return pBlock;
}

block* block::create_radio_def_block(const std::string& sName, uint uiRadioGroup)
{
    if (!has_block(sName))
    {
        bRadioChilds_ = true;
        lRadioBlockList_[uiRadioGroup] = nullptr;
        lDefBlockList_[sName] = block(sName, 0, 1, pDoc_->get_current_file_name(), pDoc_->get_current_line_nbr(), uiRadioGroup);
        block* pBlock = &lDefBlockList_[sName];
        pBlock->set_parent(this);
        pBlock->set_document(pDoc_);
        return pBlock;
    }
    else
    {
        throw utils::exception(
            pDoc_->get_current_file_name()+":"+utils::to_string(pDoc_->get_current_line_nbr())+" : "+sName_,
            "There is already a \""+sName+"\" block defined."
        );
    }
}

predefined_block* block::add_predefined_block(block* pBlock, uint uiMinNbr, uint uiMaxNbr)
{
    if (!has_block(pBlock->get_name()))
    {
        lPreDefBlockList_[pBlock->get_name()] = predefined_block(pBlock, uiMinNbr, uiMaxNbr);
        return &lPreDefBlockList_[pBlock->get_name()];
    }
    else
    {
        throw utils::exception(
            pDoc_->get_current_file_name()+":"+utils::to_string(pDoc_->get_current_line_nbr())+" : "+sName_,
            "There is already a \""+pBlock->get_name()+"\" block defined."
        );
    }
}

predefined_block* block::add_predefined_radio_block(block* pBlock, uint uiRadioGroup)
{
    if (!has_block(pBlock->get_name()))
    {
        bRadioChilds_ = true;
        lRadioBlockList_[uiRadioGroup] = nullptr;
        lPreDefBlockList_[pBlock->get_name()] = predefined_block(pBlock, 0, 1, uiRadioGroup);

        return &lPreDefBlockList_[pBlock->get_name()];
    }
    else
    {
        throw utils::exception(
            pDoc_->get_current_file_name()+":"+utils::to_string(pDoc_->get_current_line_nbr())+" : "+sName_,
            "There is already a \""+pBlock->get_name()+"\" block defined."
        );
    }
}

const std::string& block::get_file() const
{
    return sFile_;
}

uint block::get_line_nbr() const
{
    return uiLineNbr_;
}

std::string block::get_location() const
{
    return sFile_ + ":" + utils::to_string(uiLineNbr_);
}

void block::set_file(const std::string& sFile)
{
    sFile_ = sFile;
}

void block::set_line_nbr(uint uiLineNbr)
{
    uiLineNbr_ = uiLineNbr;
}
}
}
