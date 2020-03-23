#ifndef XML_BLOCK_HPP
#define XML_BLOCK_HPP

#include <string>
#include <vector>
#include <map>
#include <lxgui/utils.hpp>

namespace xml
{
    class document;
    class block;

    enum attribute_type
    {
        ATTR_TYPE_STRING,
        ATTR_TYPE_NUMBER,
        ATTR_TYPE_BOOL
    };

    /// An attribute of an XML Block
    struct attribute
    {
        attribute();
        explicit attribute(const std::string& name, bool optional = false, const std::string& def = "", attribute_type type = ATTR_TYPE_STRING);

        std::string    sName;
        std::string    sValue;
        std::string    sDefault;
        bool           bOptional;
        bool           bFound;
        attribute_type mType;
    };

    /// A wrapped pointer to a pre-defined XML Block
    struct predefined_block
    {
        predefined_block();
        predefined_block(block* block, uint min, uint max, uint radio_group = -1);

        block* pBlock;
        uint   uiMin;
        uint   uiMax;
        uint   uiRadioGroup;
    };

    /// An element in an XML file
    class block
    {
    friend class document;
    public :

        /// Default constructor.
        block();

        /// Definition constructor.
        /** \param sName         The name of this block
        *   \param uiMinNbr      The minimum number of occurences
        *   \param uiMaxNbr      The maximum number of occurences
        *   \param sFile         The file in which this block is defined
        *   \param uiLineNbr     The line at which this block is defined
        *   \param uiRadioGroup  The radio group this block belongs to
        */
        block(const std::string& sName, uint uiMinNbr, uint uiMaxNbr, const std::string& sFile, uint uiLineNbr, uint uiRadioGroup = -1);

        /// Copy constructor
        block(const block& mOther) = default;

        /// Move constructor
        block(block&& mOther) = default;

        /// Destructor.
        ~block();

        /// Copy assignment operator
        block& operator=(const block& mOther) = default;

        /// Move assignment operator
        block& operator=(block&& mOther) = default;

        /// Returns this Block's name.
        /** \return This Block's name
        */
        const std::string& get_name() const;

        /// Returns this Block's value.
        /** \return This Block's value
        *   \note Returns an empty string if none.
        */
        const std::string& get_value() const;

        /// Returns this Block's parent.
        /** \return This Block's parent
        *   \note Returns NULL for the main block.
        */
        block* get_parent() const;

        /// Returns this Block's minimum number of occurences.
        /** \return This Block's minimum number of occurences
        */
        uint get_min_count() const;

        /// Returns this Block's maximum number of occurences.
        /** \return This Block's maximum number of occurences
        */
        uint get_max_count() const;

        /// Checks if this block is a radio block.
        /** \return 'true' if this block is a radio block
        */
        bool is_radio() const;

        /// Returns the radio group this block belongs to.
        /** \return The radio group this block belongs to
        */
        uint get_radio_group() const;

        /// Checks if this block has radio children.
        /** \return 'true' if this block has radio children
        */
        bool has_radio_children() const;

        /// Returns the number of sub-blocks found in this Block.
        /** \return the number of sub-blocks found in this Block
        */
        uint get_child_number() const;

        /// Returns the number of sub-blocks with a given name found in this Block.
        /** \return the number of sub-blocks with a given name found in this Block
        */
        uint get_child_number(const std::string& sName) const;

        /// Starts iteration through this Block's sub-blocks.
        /** \param sName The name of the sub-blocks you need
        *   \return The first sub-block (NULL if none)
        *   \note If 'sName' is ommited, iteration will be
        *         done through all sub-blocks. Else, only
        *         the sub-blocks which are named 'sName'
        *         will be returned.
        */
        block* first(const std::string& sName = "");

        /// Iterates once through the sub-blocks.
        /** \return The next sub-block (NULL if none)
        *   \note See first() for more infos.
        */
        block* next();

        struct range_block
        {
            struct iterator
            {
                block* pParent = nullptr;
                block* pBlock = nullptr;

                iterator() = default;
                iterator(block* b, const std::string& n) : pParent(b) { pBlock = pParent->first(n); }
                block* operator*() { return pBlock; }
                iterator& operator++ () { pBlock = pParent->next(); return *this; }
                bool operator!= (const iterator& o) { return pBlock != o.pBlock; }
            };

            range_block(block* b, const std::string& s) : pParent(b), sName(s) {}

            block*      pParent = nullptr;
            std::string sName;

            iterator begin() { return iterator(pParent, sName); }
            iterator end() { return iterator{}; }
        };

        /// Enable iteration through this Block's sub-blocks.
        /** \param sName The name of the sub-blocks you need
        *   \return An object with begin() and end() functions.
        *   \note If 'sName' is ommited, iteration will be
        *         done through all sub-blocks. Else, only
        *         the sub-blocks which are named 'sName'
        *         will be visited.
        */
        range_block blocks(const std::string& sName = "") { return range_block{this, sName}; }

        /// Returns one of this Block's attribute.
        /** \param sName The name of the attribute you need
        *   \return One of this Block's attribute
        */
        std::string get_attribute(const std::string& sName);

        /// Checks if one of this Block's attribute has been provided.
        /** \param sName The name of the attribute to check
        *   \return 'true' if this attribute has been provided
        *   \note Calling this function is only usefull when you have
        *         optionnal attributes.
        */
        bool is_provided(const std::string& sName);

        /// Returns one of this Block's sub-block.
        /** \param sName The name of the sub-block you want
        *   \return One of this Block's sub-block
        *   \note If several sub-blocks have the same name,
        *         this function will return one of them
        *         (there is no way to predict which one).
        */
        block* get_block(const std::string& sName);

        /// Returns a radio block.
        /** \param uiGroup The radio block group
        *   \return A radio block
        *   \note Only works if this block has radio children.
        */
        block* get_radio_block(uint uiGroup = 0u);

        /// Returns the file into which this block has been found.
        /** \return The file into which this block has been found
        */
        const std::string& get_file() const;

        /// Returns the line at which this block has been found.
        /** \return The line at which this block has been found
        */
        uint get_line_nbr() const;

        /// Returns the location at which this block has been found.
        /** \return The location at which this block has been found
        *   \note Format is : <file>:<line_number>
        */
        std::string get_location() const;

    protected :

        /// Sets the file into which this block has been found.
        /** \param sFile The file into which this block has been found
        */
        void set_file(const std::string& sFile);

        /// Sets the line at which this block has been found.
        /** \param uiLineNbr The line at which this block has been found
        */
        void set_line_nbr(uint uiLineNbr);

        /// Copies a Block's data into another.
        /** \param pBlock The block to copy
        */
        void copy(block* pBlock);

        /// Adds an attribute to the list.
        /** \param mAttrib The new attribute
        *   \note Only used in the definition stage.
        */
        bool add(const attribute& mAttrib);

        /// Removes an attribute from the available list.
        /** \param sAttributeName The name of the attribute to remove
        *   \note Only used in the definition stage.
        */
        void remove_attribute(const std::string& sAttributeName);

        /// Parses attributes definitions.
        /** \param lAttribs The attribute list
        *   \note Only used in the definition stage.
        */
        void check_attributes_def(const std::vector<std::string>& lAttribs);

        /// Retrieves attributes from a string.
        /** \param sAttributes The string containing attributes
        *   \return 'false' if an attribute is missing or the synthax
        *           is incorrect
        *   \note Only used in the loading stage.
        */
        bool check_attributes(const std::string& sAttributes);

        /// Make sure all required blocks were found.
        /** \return 'false' if a required block wasn't found
        *           or if there was too much occurences of a
        *           particular block
        *   \note Only used in the loading stage.
        */
        bool check_blocks();

        /// Sets this Block's name.
        /** \param sName The new name
        *   \note Only used in definition and loading stages.
        */
        void set_name(const std::string& sName);

        /// Sets this Block's value.
        /** \param sValue The new value
        *   \note Only used in the loading stage.
        */
        void add_value(const std::string& sValue);

        /// Notify this block that another one derivates from it.
        /** \param sName The name of the derivated Block
        *   \note Only works for pre-defined blocks.<br>
        *         Only used in the definition stage.
        */
        void add_derivated(const std::string& sName);

        /// Checks if this block is being derivated from by another Block.
        /** \param sName The name of the block you want to test
        *   \note Only works for pre-defined blocks.<br>
        *         Only used in the loading stage.
        */
        bool has_derivated(const std::string& sName) const;

        /// Sets this Block's parent.
        /** \param pParent The new parent
        *   \note Only used in definition and loading stages.
        */
        void set_parent(block* pParent);

        /// Sets this Block's Document.
        /** \param pDoc The Document this block belongs to
        *   \note Only used in definition and loading stages.
        */
        void set_document(document* pDoc);

        /// Flags a radio group as optional.
        /** \param uiGroup The group ID
        */
        void set_radio_group_optional(uint uiGroup);

        /// Returns the number of sub-blocks defined for this Block.
        /** \return the number of sub-blocks defined for this Block
        *   \note Only used in definition stage.
        */
        uint get_def_child_number() const;

        /// Checks if this block has another block defined as child.
        /** \param sName The name of the block to test
        *   \return 'true' if this block already has the other one as child.
        *   \note Only used in definition stage.
        */
        bool has_block(const std::string& sName);

        /// Checks if this block can contain another Block.
        /** \param sName The name of the block to test
        *   \return 'true' if this block can contain the other one.
        *   \note Only used in loading stage.
        */
        bool can_have_block(const std::string& sName);

        /// Creates a new block in this one.
        /** \param sName The name of the block to create
        *   \return The new Block
        *   \note Only used in the loading stage.<br>
        *         Returns NULL if this block is already
        *         creating another Block.<br>
        *         Doesn't check this block can contain
        *         a new block with the provided name.
        *         See has_block().
        */
        block* create_block(const std::string& sName);

        /// Adds the previously created block to the list.
        /** \note Only used in the loading stage.<br>
        *         Call this function after create_block(),
        *         when you're finished with it.
        */
        void add_block();

        /// Creates a new block (used for definition).
        /** \param sName    The name of the block
        *   \param uiMinNbr The minimum number of occurences
        *   \param uiMaxNbr The maximum number of occurences
        *   \return The new Block
        *   \note Only used in the definition stage.
        */
        block* create_def_block(const std::string& sName, uint uiMinNbr, uint uiMaxNbr);

        /// Creates a new block (used for definition).
        /** \param sName        The name of the block
        *   \param uiRadioGroup The radio group it belongs to
        *   \return The new Block
        *   \note Only used in the definition stage.<br>
        *         Creates a "radio" block : it can only be present
        *         if none of the other radio blocks of the same
        *         group are present.
        */
        block* create_radio_def_block(const std::string& sName, uint uiRadioGroup);

        /// Adds a pre-defined block to the list.
        /** \param pBlock   The pre-defined Block
        *   \param uiMinNbr The minimum number of occurences
        *   \param uiMaxNbr The maximum number of occurences
        *   \return The new block reference
        *   \note Only used in the definition stage.
        */
        predefined_block* add_predefined_block(block* pBlock, uint uiMinNbr, uint uiMaxNbr);

        /// Adds a pre-defined block to the list.
        /** \param pBlock       The pre-defined Block
        *   \param uiRadioGroup The radio group it belongs to
        *   \return The new block reference
        *   \note Only used in the definition stage.<br>
        *         Adds a "radio" block : it can only be present
        *         if none of the other radio blocks of the same
        *         group are present.
        */
        predefined_block* add_predefined_radio_block(block* pBlock, uint uiRadioGroup);

    private :

        std::string sName_;
        uint        uiMaxNumber_;
        uint        uiMinNumber_;
        uint        uiRadioGroup_;
        bool        bRadioChilds_;
        std::string sValue_;
        document*   pDoc_;
        block*      pParent_;
        block*      pNewBlock_;
        bool        bCreating_;

        std::string sFile_;
        uint        uiLineNbr_;

        typedef std::multimap<std::string, block*>::iterator found_block_iterator;

        std::vector<found_block_iterator>::iterator mCurrIter_;
        std::vector<found_block_iterator>::iterator mEndIter_;

        std::vector<std::string> lDerivatedList_;

        std::map<std::string, attribute>        lAttributeList_;
        std::map<std::string, block>            lDefBlockList_;
        std::map<std::string, predefined_block> lPreDefBlockList_;

        std::map<uint, block*> lRadioBlockList_;
        std::vector<uint>      lOptionalRadioGroupList_;

        std::multimap<std::string, block*>                       lFoundBlockList_;
        std::vector<found_block_iterator>                        lFoundBlockStack_;
        std::map<std::string, std::vector<found_block_iterator>> lFoundBlockSortedStacks_;
    };
}

#endif
