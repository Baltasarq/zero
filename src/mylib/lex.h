// lex.h
/*
	Lexical analyzer.

	baltasarq@yahoo.es
*/

#ifndef _LEX_H
#define _LEX_H

#include "stringman.h"
#include "fileio.h"

#include <string>
#include <stdexcept>
#include <memory>

/***
	Class Lexer.
	This class defines the minimum capabilities of any lexer.
*/

class Lexer {
public:
    /// Literal types
    typedef enum {
        Nothing,
        Id,
        Number,
        Text,
        SpecialCharacter,
        Delimiter
    } TokenType;
    static const std::string StandardDelimiters;

    // Minimum interface
    virtual const std::string &getLine()        = 0;
    virtual std::string &getCurrentToken()      = 0;
    virtual std::string &getToken()             = 0;
    virtual std::string &getNumber()            = 0;
    virtual TokenType getNextTokenType()        = 0;
    virtual TokenType getCurrentTokenType()     = 0;
    virtual char getCurrentChar()               = 0;
    virtual void skipDelim()                    = 0;
    virtual std::string &getLiteral(const std::string &) = 0;
    virtual unsigned int getCurrentPos()        = 0;
    virtual void reset(unsigned int i = 0)      = 0;
    virtual void advance(int = 1)               = 0;
    virtual bool isEnd()                        = 0;

    virtual ~Lexer() {}

    // Analyzing tools
	static std::string &getNumber(const std::string &lin, unsigned int &pos, std::string &num, const std::string &);
	static std::string &getToken(const std::string &lin, unsigned int &pos, std::string &token, const std::string &delims);
	static void skipDelim(const std::string &lin, unsigned int &pos, const std::string &delims, int avance = 1);
	static void skipSpaces(const std::string &lin, unsigned int &pos, int avance = 1);
    static TokenType getTokenType(int ch, const std::string &delims);
    static std::string &getLiteral(const std::string &lin,
				    unsigned int &pos,
				    const std::string &delim,
				    std::string &lit);
};

/***
	Class StringLexer.
	This class provides lexical analyzing capabilities in a string.
*/

class StringLexer : public Lexer {
public:
    /// Builds a lexer, copying the string to analyze
	StringLexer(std::string *x, const std::string &delims = "");

	/// Current line
	const std::string &getLine() const { return *txt; };
	std::string &getLine() { return *txt; };

	/// Current token
	std::string &getCurrentToken()
        { return token; };

    /// Current token type
    TokenType getCurrentTokenType()
        { return Lexer::getTokenType( getCurrentChar(), delimiters.c_str() ); }

	/// Return next token to come
	/// Use getNextTokenType in order to know whether there is a literal or not.
	/// @see getNextTokenType
	std::string &getToken()
        { return Lexer::getToken( *txt, pos, token, delimiters ); }

	/// Return next number to come
	/// Use getNextTokenType in order to know whether there is a literal or not.
	/// @see getNextTokenType
	std::string &getNumber()
        { return Lexer::getNumber( *txt, pos, token, delimiters ); }

	/// Skip delimiters from current pos
	void skipDelim()
        { Lexer::skipDelim( *txt, pos, delimiters ); }

	/// Skips spaces and tabs
	void skipSpaces()
        { Lexer::skipSpaces( *txt, pos ); }

	/// Return type of the next token without actually reading it
	TokenType getNextTokenType();

	/// Return next literal
	std::string &getLiteral(const std::string &);

	/// end of line?
	bool isEol() const
        { return ( pos >= txt->length() ); }
	bool isEnd()
        { return isEol(); }

	/// Advance in text
	void advance(int = 1);

	/// Return current position
	unsigned int getCurrentPos()
        { return pos; }

	/// Resets current position value
	void reset(unsigned int i = 0)
        { pos = i; }

	/// Returns the next char to be considered
	char getCurrentChar();


    void setToken(const std::string &token)
        { this->token = token; }

    void appendToToken(const std::string &contents)
        { this->token.append( contents ); }

private:
    std::string delimiters;
	unsigned int pos;
	std::string * txt;
	std::string token;
};


/***
	Class FileLexer.
	This class provides lexical analyzing capabilities.
*/

class FileLexer : public Lexer {
public:
	FileLexer(const std::string &fileName, const std::string &delims = "");

	/// Current line
	std::string &getLine()
        { getNextLineFromFile(); return lex->getLine(); };

    /// Current line (synonim of getLine)
    /// @see getLine
    std::string &getCurrentLine()
        { return getLine(); };

    /// Current line number
    unsigned int getLineNumber() const
        { return num; }

    /// file name
    const std::string &getFileName() const
        { return f.getFileName(); }

	/// Current token
	std::string &getCurrentToken()
        { getNextLineFromFile(); return lex->getCurrentToken(); };

	/// Return next token
	std::string &getToken()
        { getNextLineFromFile(); return lex->getToken(); }

	/// Return next number
	std::string &getNumber()
        { getNextLineFromFile(); return lex->getNumber(); }

	/// Skip delimiters from current pos
	void skipDelim()
        { getNextLineFromFile(); return lex->skipDelim(); }

	/// Return type of the next token without actually reading it
	TokenType getNextTokenType()
        { getNextLineFromFile(); return lex->getNextTokenType(); }

    /// Return next literal (multi-line literal)
	std::string &getLiteral(const std::string &);

	/// end of file?
	bool isEof() const
        { return f.isEof(); }
	bool isEnd()
        { getNextLineFromFile(); return ( f.isEof() && lex->isEol() ); }

    /// Reset to a given position (in current line)
    void reset(unsigned int i = 0)
        { lex->reset( i ); getNextLineFromFile(); }

	/// Advance in text
	void advance(int i = 1)
        { getNextLineFromFile(); lex->advance( i ); }

	/// Return current position (in current line)
	unsigned int getCurrentPos()
        { getNextLineFromFile(); return lex->getCurrentPos(); }

	/// Returns the next char to be considered
	char getCurrentChar()
        { getNextLineFromFile(); return lex->getCurrentChar(); }

    /// The type of the current char
    TokenType getCurrentTokenType()
        { getNextLineFromFile(); return lex->getCurrentTokenType(); }

    /// Skip current line
    void skipLine()
        { readNextLineFromFile(); }

    /// @brief How many blank lines were skipped last read
    /// How many blank lines were skipped last read, provided an EOL was found.
    /// @return An integer with the last blank lines skipped. This value does not
    /// change until next EOL
    /// @see wasEol
    unsigned int getNumBlankLinesSkipped() const
        { return numLinesSkipped; }

    /// @brief Was an EOL find before reading the last char ?
    /// @return true if it was, or false otherwise
    /// @see getNumBlankLinesSkipped
    bool wasEol() const
        { return (
                lex->getCurrentPos() == 0
             && f.getPos() > 0
             && !isEof()
          );
        }

private:
    std::string lin;
	std::auto_ptr<StringLexer> lex;
	std::string delimiters;
	InputFile f;
	unsigned int num;
	unsigned int numLinesSkipped;

    /// Loads the next line from file if needed
	void getNextLineFromFile();

    /// Mandatorily reads the next line from file
    /// @see getNextLineFromFile
    void readNextLineFromFile();

    /// Checks whether a delimiter was found or not in the last getLiteral()
    /// @see getLiteral
    bool delimiterFound(StringLexer &l, const std::string &delim);
};


#endif
