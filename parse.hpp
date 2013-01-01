#pragma once

#include <deque>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

/** Implements parser state for an Egg parser.
 *  
 *  @author Aaron Moss
 */

namespace parse {
	
	typedef unsigned long	ind;	/**< unsigned index type */
	
	
	/** Error thrown when a parser is asked for state it has forgotten. */
	struct forgotten_state_error : public std::range_error {
		
		/** Default constructor.
		 *  @param req		Requested index
		 *  @param avail	Minimum available index
		 */
		forgotten_state_error(ind req, ind avail) throw() 
			: std::range_error("Forgotten state error"), 
			req(req), avail(avail) {}
		
		/** Inherited from std::exception. */
		const char* what() const throw() {
			try {
				std::stringstream ss("Forgotten state error");
				ss << ": requested " << req << " < " << avail;
				return ss.str().c_str();
			} catch (std::exception const& e) {
				return "Forgotten state error";
			}
		}
		
		/** requested index */
		ind req;
		/** minimum available index */
		ind avail;
	}; /* struct forgotten_range_error */
	
	/** Parser state */
	class state {
	public:
		typedef char								value_type;
		typedef std::deque<char>::iterator			iterator;
		typedef std::pair<iterator, iterator>		range_type;
		typedef std::deque<char>::difference_type	difference_type;
		typedef ind								size_type;
		
		/** Default constructor.
		 *  Initializes state at beginning of input stream.
		 *  @param in		The input stream to read from
		 */
		state(std::istream& in) : pos(0), str(), str_off(0), in(in) {}
		
		/** Indexing operator.
		 *  Returns character at specified position in the input stream, 
		 *  reading more input if necessary.
		 *  @param i		The index of the character to get
		 *  @return The i'th character of the input stream, or '\0' for i past 
		 *  		the end of file
		 *  @throws forgotten_state_error on i < str_begin (that is, asking for 
		 *  		input previously discarded)
		 */
		value_type operator[] (size_type i) {
			
			// Fail on forgotten index
			if ( i < str_off ) throw forgotten_state_error(i, str_off);
			
			// Get index into stored input
			ind ii = i - str_off;
			
			// Expand stored input if needed
			if ( ii >= str.size() ) {
				ind n = 1 + ii - str.size();
				ind r = read(n);
				if ( r < n ) return '\0';
			}
			
			return str[ii];
		}
		
		/** Range operator.
		 *  Returns a pair of iterators, begin and end, containing up to the 
		 *  given number of elements, starting at the given index. The end 
		 *  iterator is not guaranteed to be dereferencable, though any 
		 *  iterator between it and the begin iterator (inclusive) is 
		 *  dereferencable. Returned begin and end iterators may be invalidated 
		 *  by calls to any non-const method of this class.
		 *  @param i		The index of the beginning of the range
		 *  @param n		The maximum number of elements in the range
		 *  @throws forgotten_state_error on i < str_begin (that is, asking for 
		 *  		input previously discarded)
		 */
		range_type range(size_type i, size_type n) {
			
			// Fail on forgotten index
			if ( i < str_off ) throw forgotten_state_error(i, str_off);
			
			// Get index into stored input
			ind ib = i - str_off;
			ind ie = ib + n;
			
			// Expand stored input if needed
			if ( ie > str.size() ) {
				ind nn = ie - str.size();
				read(nn);
			}
			
			// Get iterators, adjusting for the end of the input
			iterator bIter, eIter;
			
			if ( ie >= str.size() ) {
				eIter = str.end();
				if ( ib >= str.size() ) {
					bIter = str.end();
				} else {
					bIter = str.begin() + ib;
				}
			} else {
				bIter = str.begin() + ib;
				eIter = str.begin() + ie;
			}
			
			return range_type(bIter, eIter);
		}
		
		/** Substring operator.
		 *  Convenience for the string formed by the characters in range(i, n).
		 *  @param i		The index of the beginning of the range
		 *  @param n		The maximum number of elements in the range
		 *  @throws forgotten_state_error on i < str_begin (that is, asking for 
		 *  		input previously discarded)
		 */
		std::string string(size_type i, size_type n) {
			range_type iters = range(i, n);
			return std::string(iters.first, iters.second);
		}
		
		/** Forgets all parsing state before the given index.
		 *  After this, reads or indexes before the given index will fail with 
		 *  an exception.
		 *  @param i		The index to forget to
		 */
		void forgetTo(size_type i) {
			
			// Ignore if already forgotten
			if ( i <= str_off ) return;
			
			// Get index in stored input to forget
			ind ii = i - str_off;
			
			// Forget stored input
			str.erase(str.begin(), str.begin()+ii);
		}
		
	private:
		/** Read more characters into the parser.
		 *  @param n		The number of characters to read
		 *  @return The number of characters read
		 */
		size_type read(size_type n) {
			char s[n];
			// Read into buffer
			in.read(s, n);
			// Count read characters
			ind r = in.gcount();
			// Add to stored input
			str.insert(str.end(), s, s+r);
			return r;
		}
		
	public:
		/** Current parsing location */
		size_type pos;
		
	private:
		/** Characters currently in use by the parser */
		std::deque<value_type> str;
		/** Offset of start of str from the beginning of the stream */
		size_type str_off;
		/** Input stream to read characters from */
		std::istream& in;
	}; /* class state */

	/** A generic parse result. */
	struct value {
		value() {}
		value(int) {}
	};

	/** A generic unsuccessful parse result. */
	struct failure {
		failure() {}
		failure(int) {}
	};

	/** A value instance */
	const value val = 0;
	
	/** A failure instance */
	const failure fails = 0;

	/** Wraps a parsing result. ps[ps.pos++]
	 *  Returns either the wrapped result or false.
	 *  @param T The wrapped result; should be default constructable. */
	template<typename T> 
	class result {
	public:
		result(const T& v) : val(v), success(true) {}
		result(const failure& f) : success(false) {}
		result() : success(false) {}

		result<T>& operator = (const T& v) { 
			val = v; success = true; return *this;
		}
		result<T>& operator = (const failure& f) { 
			success = false; return *this;
		}
		result<T>& operator = (const result<T>& o) {
			if ( o.success ) { success = true; val = o.val; }
			else { success = false; }
			return *this;
		}

		operator T () { return val; }
		operator bool () { return success; }

		T operator * () { return val; }
		
	private:
		T val;			/**< The wrapped value. */
		bool success;	/**< The success of the parse. */
	}; /* class result<T> */

	/** Builds a positive result from a value.
	 *  @param T	The type of the wrapped result	
	 *  @param v	The value to wrap. */
	template<typename T>
	result<T> match(const T& v) { return result<T>(v); }

	/** Builds a failure result.
	 *  @param T	The type of the failure result. */
	template<typename T>
	result<T> fail() { return result<T>(fails); }

	/** Binds the result of a matcher to a local variable.
	 *  @param T		The type of the result.
	 *  @param matcher	The matcher to bind
	 *  @param ps		The current parse state
	 *  @param val		The value to return on successful match
	 */
	template<typename T>
	bool bind(result<T> (*matcher)(state&), state& ps, T& val) {
		result<T> res = matcher(ps);
		if ( res ) { val = *res; return true; }
		else { return false; }
	}
	
	/** Matcher for any character */
	result<state::value_type> any(state& ps) {
		if ( ps[ps.pos] == '\0' ) return fail<state::value_type>();
		return match(ps[ps.pos++]);
	}

	/** Matcher for a given character */
	template<state::value_type c>
	result<state::value_type> matches(parse::state& ps) {
		if ( ps[ps.pos] != c ) return fail<state::value_type>();
		++ps.pos;
		return match(c);
	}

	/** Matcher for a character range */
	template<state::value_type s, state::value_type e>
	result<std::pair<state::value_type, state::value_type>> 
			in_range(parse::state& ps) {
		state::value_type c = ps[ps.pos];
		if ( c < s || c > e ) 
			return fail<std::pair<state::value_type, state::value_type>>();
		
		++ps.pos;
		return match(std::make_pair(s,e));
	}
	
} /* namespace parse */

