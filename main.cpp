/*
 * Copyright (c) 2013 Aaron Moss
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "egg.hpp"
#include "parse.hpp"
#include "visitors/compiler.hpp"
#include "visitors/normalizer.hpp"
#include "visitors/printer.hpp"

/** Egg version */
static const char* VERSION = "0.1.0";

/** Egg usage string */
static const char* USAGE = 
"[-c print|compile] [-i input_file] [-o output_file] [--no-norm] [--help] [--version] [--usage]";

/** Full Egg help string */
static const char* HELP = 
"egg [command] [flags] [input-file [output-file]]\n\
\n\
Supported flags are\n\
 -i --input    input file (default stdin)\n\
 -o --output   output file (default stdout)\n\
 -c --command  command - either compile, print, help, usage, or version\n\
               (default compile)\n\
 -n --name     grammar name - if none given, takes the longest prefix of\n\
               the input or output file name (output preferred) which is a\n\
               valid Egg identifier (default empty)\n\
 --no-norm     turns off grammar normalization\n\
 --usage       print usage message\n\
 --help        print full help message\n\
 --version     print version string\n";

/** Command to run */
enum egg_mode {
	PRINT_MODE,		/**< Print grammar */
	COMPILE_MODE,	/**< Compile grammar */
	USAGE_MODE,     /**< Print usage */
	HELP_MODE,      /**< Print help */
	VERSION_MODE    /**< Print version */
};

class args {
private:
	bool eq(const char* lit, char* arg) {
		return std::string(lit) == std::string(arg);
	}

	bool match(const char* shrt, const char* lng, char* arg) {
		std::string a(arg);
		return std::string(shrt) == a || std::string(lng) == a;
	}

	std::string id_prefix(char* s) {
		int len = 0;
		char c = s[len];
		if ( ( c >= 'A' && c <= 'Z' )
				|| ( c >= 'a' && c <= 'z' )
				|| c == '_' ) {
			c = s[++len];
		} else {
			return std::string("");
		}

		while ( ( c >= 'A' && c <= 'Z' )
				|| ( c >= 'a' && c <= 'z' )
				|| ( c >= '0' && c <= '9' )
				|| c == '_' ) {
			c = s[++len];
		}

		return std::string(s, len);
	}

	bool parse_mode(char* s) {
		if ( eq("print", s) ) {
			eMode = PRINT_MODE;
			return true;
		} else if ( eq("compile", s) ) {
			eMode = COMPILE_MODE;
			return true;
		} else if ( eq("help", s) ) {
			eMode = HELP_MODE;
			return true;
		} else if ( eq("usage", s) ) {
			eMode = USAGE_MODE;
			return true;
		} else if ( eq("version", s) ) {
			eMode = VERSION_MODE;
			return true;
		} else {
			return false;
		}
	}

	void parse_input(char* s) {
		in = new std::ifstream(s);
		if ( !nameFlag && out == 0 ) {
			pName = id_prefix(s);
		}
	}

	void parse_output(char* s) {
		out = new std::ofstream(s);
		if ( !nameFlag ) {
			pName = id_prefix(s);
		}
	}

	void parse_name(char* s) {
		pName = id_prefix(s);
		nameFlag = true;
	}

public:
	args(int argc, char** argv) {
		in = (std::ifstream*)0;
		out = (std::ofstream*)0;
		pName = std::string("");
		nameFlag = false;
		normFlag = true;
		eMode = COMPILE_MODE;

		i = 1;
		if ( argc <= 1 ) return;

		//parse optional sub-command
		if ( parse_mode(argv[i]) ) { ++i; }

		//parse explicit flags
		for (; i < argc; ++i) {
			if ( match("-i", "--input", argv[i]) ) {
				if ( i+1 >= argc ) return;
				parse_input(argv[++i]);
			} else if ( match("-o", "--output", argv[i]) ) {
				if ( i+1 >= argc ) return;
				parse_output(argv[++i]);
			} else if ( match("-c", "--command", argv[i]) ) {
				if ( i+1 >= argc ) return;
				if ( parse_mode(argv[++i]) ) { ++i; }
			} else if ( match("-n", "--name", argv[i]) ) {
				if ( i+1 >= argc ) return;
				parse_name(argv[++i]);
			} else if ( eq("--no-norm", argv[i]) ) {
				normFlag = false;
			} else if ( eq("--usage", argv[i]) ) {
				eMode = USAGE_MODE;
			} else if ( eq("--help", argv[i]) ) {
				eMode = HELP_MODE;
			} else if ( eq("--version", argv[i]) ) {
				eMode = VERSION_MODE;
			} else break;
		}

		//parse optional input and output files
		if ( i < argc ) {
			parse_input(argv[i++]);
			if ( i < argc ) {
				parse_output(argv[i++]);
			}
		}
	}

	~args() {
		if ( in != 0 ) in->close();
	}

	std::istream& input() { if ( in ) return *in; else return std::cin; }
	std::ostream& output() { if ( out ) return *out; else return std::cout; }
	std::string name() { return pName; }
	bool norm() { return normFlag; }
	egg_mode mode() { return eMode; }

private:
	int i;				/**< next unparsed value */
	std::ifstream* in;	/**< pointer to input stream (0 for stdin) */
	std::ofstream* out;	/**< pointer to output stream (0 for stdout) */
	std::string pName;	/**< the name of the parser (empty if none) */
	bool nameFlag;		/**< has the parser name been explicitly set? */
	bool normFlag;      /**< should egg do grammar normalization? */
	egg_mode eMode;		/**< compiler mode to use */
};

/** Command line interface
 *  egg [command] [flags] [input-file [output-file]]
 *  
 *  Supported flags are
 *  -i --input    input file (default stdin)
 *  -o --output   output file (default stdout)
 *  -c --command  command - either compile, print, help, usage, or version 
 *                (default compile)
 *  -n --name     grammar name - if none given, takes the longest prefix of 
 *                the input or output file name (output preferred) which is a 
 *                valid Egg identifier (default empty)
 *  --no-norm     turns off grammar normalization
 *  --usage       print usage message
 *  --help        print full help message
 *  --version     print version string
 */
int main(int argc, char** argv) {

	args a(argc, argv);
	
	switch ( a.mode() ) {
	case USAGE_MODE:
		std::cout << argv[0] << " " << USAGE << std::endl;
		return 0;
	case HELP_MODE:
		std::cout << HELP << std::endl;
		return 0;
	case VERSION_MODE:
		std::cout << "Egg version " << VERSION << std::endl;
		return 0;
	default: break;
	}

	parse::state ps(a.input());
	ast::grammar_ptr g;
	
	if ( egg::grammar(ps)(g) ) {
		//std::cout << "DONE PARSING" << std::endl;
		if ( a.norm() ) {
			visitor::normalizer n;
			n.normalize(*g);
		}

		switch ( a.mode() ) {
		case PRINT_MODE: {
			visitor::printer p(a.output());
			p.print(*g);
			break;
		} case COMPILE_MODE: {
			visitor::compiler c(a.name(), a.output());
			c.compile(*g);
			break;
		}
		default: break;
		}
		
	} else {
		int64_t maxPos=ps.maxRead();
		int64_t startPos; for (startPos=maxPos-1;startPos>0&&ps[startPos]!='\n';--startPos); startPos+=ps[startPos]=='\n';
		int64_t endPos; for (endPos=maxPos;ps[endPos]!='\n'&&ps[endPos]!='\0';++endPos);
		int64_t lineCount=1;
		try {
			for(int64_t pos=startPos;pos>0;--pos) lineCount+=(ps[pos]=='\n');
		} catch(parse::forgotten_state_error& e) {
			lineCount += e.newlines;
		}
		int64_t errPos=maxPos-startPos;

		std::cerr << "Parse failure " << maxPos << " bytes into the input:" << std::endl;
		std::cerr << "line " << lineCount << ":   " << ps.string(startPos,endPos-startPos) << std::endl;
		std::cerr << std::string(5+ceil(log10(lineCount)+4),' ') << std::string(maxPos-startPos-1, ' ') << "^-- error, column " << errPos << std::endl;
		return 1;
	}

	return 0;
} /* main() */
