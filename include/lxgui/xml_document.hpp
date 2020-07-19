#ifndef LXGUI_XML_DOCUMENT_HPP
#define LXGUI_XML_DOCUMENT_HPP

#include "lxgui/xml_block.hpp"
#include <iostream>

namespace lxgui {
namespace xml
{
    /// Parses an XML file
    /** Uses a definition file (*.def) to validate
    *   the parsed file.
    */
    class document
    {
    friend class block;
    public :

        /// Default constructor.
        /** \param sDefFileName The path to the definition file to use
        *   \param mOut         The output stream for errors and warnings
        *   \note The definition file is parsed in this constructor.
        */
        explicit document(const std::string& sDefFileName, std::ostream& mOut = std::cout);

        /// File constructor.
        /** \param sFileName    The path to the file you want to parse
        *   \param sDefFileName The path to the definition file to use
        *   \param mOut         The output stream for errors and warnings
        *   \note The definition file is parsed in this constructor.
        */
        document(const std::string& sFileName, const std::string& sDefFileName, std::ostream& mOut = std::cout);

        document(const document&) = delete;
        document(document&&) = delete;
        document& operator=(const document&) = delete;
        document& operator=(document&&) = delete;

        /// Sets the file from which to read data from.
        /** \param sFileName The path to the file you want to parse
        */
        void set_file_name(const std::string& sFileName);

        /// Sets the string from which to read data from.
        /** \param sString The string containing data to parse
        */
        void set_source_string(const std::string& sString);

        /// Returns this file's main block.
        /** \return This file's main block
        */
        block* get_main_block();

        /// Returns this file's name.
        /** \return This file's name
        *   \note Only used internaly.
        */
        const std::string& get_current_file_name() const;

        /// Returns the line that is being parsed.
        /** \return The line that is being parsed
        *   \note Only used internaly.<br>If you call this
        *         method before check(), it will return the
        *         total number of line in the definition file.
        *         After, it will return the total number of
        *         line in the XML file.
        */
        uint get_current_line_nbr() const;

        /// Returns the current parsing location.
        /** \return The current parsing location
        *   \note Format : filename:line
        */
        std::string get_current_location() const;

        /// Parses and validates the XML file.
        /** \param sPreProcCommands Preprocessor commands to use when parsing
        *                           the xml file
        *   \return 'true' if everything went fine
        *   \note This function takes care of retreiving
        *         values from the file and also Checks the
        *         content of the file acording to the *.def
        *         file you provided.<br>
        *         The preprocessor commands must be separated by commas ','.
        */
        bool check(const std::string& sPreProcCommands = "");

    protected :

        /// Creates a new predefined block with inheritance.
        /** \param sName        The name of the new predefined Block
        *   \param sInheritance The name of the inherited Block
        */
        void create_predefined_block(const std::string& sName, const std::string& sInheritance = "");

        /// Returns a predefined Block.
        /** \param sName The name of the predefined block you want
        *   \note Only used in loading stage.
        */
        const block* get_predefined_block(const std::string& sName) const;

        /// Returns a predefined Block.
        /** \param sName The name of the predefined block you want
        *   \note Only used in loading stage.
        */
        block* get_predefined_block(const std::string& sName);

        /// Flags this Document as invalid.
        void set_invalid();

        std::ostream out;

    private :

        /** \cond NOT_REMOVE_FROM_DOC
        */
        class state
        {
        public :

            enum class id
            {
                DEF,
                XML
            };

            state();

            virtual ~state();

            virtual std::string read_tag_name(const std::string& sTagContent) const = 0;
            virtual void  read_opening_tag(const std::string& sTagContent) = 0;
            virtual void  read_single_tag(const std::string& sTagContent) = 0;
            virtual void  read_ending_tag(const std::string& sTagContent) = 0;

            void          set_document(document* pDoc);
            void          set_current_block(block* pBlock);
            void          set_current_parent_block(block* pParentBlock);
            void          add_content(const std::string& sContent);
            const id&     get_id() const;

        protected :

            document* pDoc_;
            id        mID_;

            block*      pCurrentBlock_;
            block*      pCurrentParentBlock_;
        };
        friend class state;

        class xml_state : public state
        {
        public :

            xml_state();

            std::string read_tag_name(const std::string& sTagContent) const;
            void  read_opening_tag(const std::string& sTagContent);
            void  read_single_tag(const std::string& sTagContent);
            void  read_ending_tag(const std::string& sTagContent);
        };
        friend class xml_state;

        class def_state : public state
        {
        public :

            def_state();

            std::string read_tag_name(const std::string& sTagContent) const;
            void  read_opening_tag(const std::string& sTagContent);
            void  read_single_tag(const std::string& sTagContent);
            void  read_ending_tag(const std::string& sTagContent);

        private :

            void read_predef_commands_(
                std::string& sName, std::string& sParent, uint& uiMin, uint& uiMax,
                bool& bCopy, bool& bPreDefining, bool& bLoad, uint& uiRadioGroup,
                bool bMultiline
            );
        };
        friend class def_state;
        /** \endcond
        */

        void load_definition_();

        void read_tags_(std::string& sLine);
        void read_opening_tag_(std::string& sTagContent);
        void read_single_tag_(std::string& sTagContent);
        void read_ending_tag_(std::string& sTagContent);

        std::string  sFileName_;
        std::string  sSourceString_;
        std::string  sDefFileName_;
        std::string  sCurrentFileName_;
        uint         uiCurrentLineNbr_ = 0;
        bool         bValid_ = true;

        xml_state mXMLState_;
        def_state mDefState_;
        state*    pState_ = nullptr;

        // Load state
        std::vector<std::string> lPreProcessorCommands_;
        bool        bSmartComment_ = false;
        std::string sSmartCommentTag_;
        uint        uiSmartCommentCount_ = 0u;
        bool        bMultilineComment_ = false;
        bool        bPreProcessor_ = false;
        uint        uiPreProcessorCount_ = 0u;
        uint        uiSkippedPreProcessorCount_ = 0u;

        block mMainBlock_;

        std::map<std::string, block> lPredefinedBlockList_;
    };
}
}

#endif
