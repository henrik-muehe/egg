#pragma once

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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/strings.hpp"

namespace ast {
	using std::string;
	using std::unordered_map;
	using std::vector;
	using std::shared_ptr;

	template<typename T, typename... Args>
	shared_ptr<T> make_ptr(Args... args) { return std::make_shared<T>(args...); }

	template<typename T, typename U>
	shared_ptr<T> as_ptr(const shared_ptr<U>& r) { return std::static_pointer_cast<T>(r); }

	/** Represents a character range. */
	class char_range {
	public:
		char_range(char from, char to) : from(from), to(to) {}
		char_range(char c) : from(c), to(c) {}
		char_range(const char_range& o) : from(o.from), to(o.to) {}
		char_range() : from('\0'), to('\0') {}

		bool single() { return from == to; }

		char from;	/**< The first character in the range */
		char to;	/**< The last character in the range. If this is the same 
					 *   as the first character, represents a single character 
					 */
	}; /* class char_range */
	typedef shared_ptr<char_range> char_range_ptr;

	class char_matcher;
	class str_matcher;
	class range_matcher;
	class rule_matcher;
	class any_matcher;
	class empty_matcher;
	class action_matcher;
	class opt_matcher;
	class many_matcher;
	class some_matcher;
	class seq_matcher;
	class alt_matcher;
	class look_matcher;
	class not_matcher;
	class capt_matcher;

	/** Type of AST node. */
	enum matcher_type {
		char_type,
		str_type,
		range_type,
		rule_type,
		any_type,
		empty_type,
		action_type,
		opt_type,
		many_type,
		some_type,
		seq_type,
		alt_type,
		look_type,
		not_type,
		capt_type
	}; /* enum matcher_type */
	
	/** Abstract base class of all matcher visitors.
	 *  Implements visitor pattern. */
	class visitor {
	public:
		virtual void visit(char_matcher&) = 0;
		virtual void visit(str_matcher&) = 0;
		virtual void visit(range_matcher&) = 0;
		virtual void visit(rule_matcher&) = 0;
		virtual void visit(any_matcher&) = 0;
		virtual void visit(empty_matcher&) = 0;
		virtual void visit(action_matcher&) = 0;
		virtual void visit(opt_matcher&) = 0;
		virtual void visit(many_matcher&) = 0;
		virtual void visit(some_matcher&) = 0;
		virtual void visit(seq_matcher&) = 0;
		virtual void visit(alt_matcher&) = 0;
		virtual void visit(look_matcher&) = 0;
		virtual void visit(not_matcher&) = 0;
		virtual void visit(capt_matcher&) = 0;
	}; /* class visitor */
	
	/** Abstract base class of all matchers.
	 *  Implements visitor pattern. */
	class matcher {
	public:
		/** Implements visitor pattern. */
		virtual void accept(visitor*) = 0;
		/** Gets type tag. */
		virtual matcher_type type() = 0;
	}; /* class matcher */
	typedef shared_ptr<matcher> matcher_ptr;
	
	/** Matches a character literal. */
	class char_matcher : public matcher {
	public:
		char_matcher(char c) : c(c) {}
		char_matcher() : c('\0') {}
		
		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return char_type; }
		
		char c; /**< char to match */
	}; /* class char_matcher */
	typedef shared_ptr<char_matcher> char_matcher_ptr;

	/** Matches a string literal. */
	class str_matcher : public matcher {
	public:
		str_matcher(string s) : s(s) {}
		str_matcher() : s("") {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return str_type; }

		string s; /**< string to match */
	}; /* class str_matcher */
	typedef shared_ptr<str_matcher> str_matcher_ptr;

	/** Matches a character range. */
	class range_matcher : public matcher {
	public:
		range_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return range_type; }

		range_matcher& operator += (char_range r) { rs.push_back(r); return *this; }

		vector<char_range> rs; /**< contained character ranges */
	}; /* class range_matcher */
	typedef shared_ptr<range_matcher> range_matcher_ptr;

	/** Matches a grammar rule invocation. */
	class rule_matcher : public matcher {
	public:
		rule_matcher(string rule) : rule(rule), var("") {}
		rule_matcher(string rule, string var) : rule(rule), var(var) {}
		rule_matcher() : rule(""), var("") {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return rule_type; }

		string rule;	/**< The name of the rule to match */
		string var;		/**< Variable to bind to the rule return. 
						 *   Empty if unset. */
	}; /* class rule_matcher */
	typedef shared_ptr<rule_matcher> rule_matcher_ptr;

	/** Matches any character. */
	class any_matcher : public matcher {
	public:
		any_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return any_type; }
	}; /* class any_matcher */
	typedef shared_ptr<any_matcher> any_matcher_ptr;

	/** Always matches without consuming a character. */
	class empty_matcher : public matcher {
	public:
		empty_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return empty_type; }
	}; /* class empty_matcher */
	typedef shared_ptr<empty_matcher> empty_matcher_ptr;

	/** Semantic action; not actually a matcher. */
	class action_matcher : public matcher {
	public:
		action_matcher(string a) : a(a) {}
		action_matcher() : a("") {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return action_type; }

		string a; /**< The string representing the action */
	}; /* class action_matcher */
	typedef shared_ptr<action_matcher> action_matcher_ptr;

	/** An optional matcher */
	class opt_matcher : public matcher {
	public:
		opt_matcher(shared_ptr<matcher> m) : m(m) {}
		opt_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return opt_type; }

		shared_ptr<matcher> m; /**< contained matcher */
	}; /* class opt_matcher */
	typedef shared_ptr<opt_matcher> opt_matcher_ptr;

	/** Matches any number of times */
	class many_matcher : public matcher {
	public:
		many_matcher(shared_ptr<matcher> m) : m(m) {}
		many_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return many_type; }

		shared_ptr<matcher> m; /**< contained matcher */
	}; /* class many_matcher */
	typedef shared_ptr<many_matcher> many_matcher_ptr;

	/** Matches some non-zero number of times */
	class some_matcher : public matcher {
	public:
		some_matcher(shared_ptr<matcher> m) : m(m) {}
		some_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return some_type; }

		shared_ptr<matcher> m; /**< contained matcher */
	}; /* class some_matcher */
	typedef shared_ptr<some_matcher> some_matcher_ptr;

	/** Sequence of matchers. */
	class seq_matcher : public matcher {
	public:
		seq_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return seq_type; }

		seq_matcher& operator += (shared_ptr<matcher> m) { ms.push_back(m); return *this; }

		vector<shared_ptr<matcher>> ms; /**< The matchers in the sequence */
	}; /* class seq_matcher */
	typedef shared_ptr<seq_matcher> seq_matcher_ptr;

	/** Alternation matcher. */
	class alt_matcher : public matcher {
	public:
		alt_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return alt_type; }

		alt_matcher& operator += (shared_ptr<matcher> m) { ms.push_back(m); return *this; }

		vector<shared_ptr<matcher>> ms; /**< The alternate matchers */
	}; /* class alt_matcher */
	typedef shared_ptr<alt_matcher> alt_matcher_ptr;

	/** Lookahead matcher. */
	class look_matcher : public matcher {
	public:
		look_matcher(shared_ptr<matcher> m) : m(m) {}
		look_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return look_type; }

		shared_ptr<matcher> m; /**< The matcher to check on lookahead */
	}; /* class look_matcher */
	typedef shared_ptr<look_matcher> look_matcher_ptr;

	/** Negative lookahead matcher. */
	class not_matcher : public matcher {
	public:
		not_matcher(shared_ptr<matcher> m) : m(m) {}
		not_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return not_type; }

		shared_ptr<matcher> m; /**< The matcher to check on lookahead */
	}; /* class not_matcher */
	typedef shared_ptr<not_matcher> not_matcher_ptr;

	/** String-capturing matcher. */
	class capt_matcher : public matcher {
	public:
		capt_matcher(shared_ptr<matcher> m) : m(m) {}
		capt_matcher() {}

		void accept(visitor* v) { v->visit(*this); }
		matcher_type type() { return capt_type; }

		shared_ptr<matcher> m; /**< Captured matcher */
	}; /* class capt_matcher */
	typedef shared_ptr<capt_matcher> capt_matcher_ptr;

	/** Empty visitor class; provides a default implementation of each of the 
	 *  methods. */
	class default_visitor : visitor {
	public:
		void visit(char_matcher& m) {}
		void visit(str_matcher& m) {}
		void visit(range_matcher& m) {}
		void visit(rule_matcher& m) {}
		void visit(any_matcher& m) {}
		void visit(empty_matcher& m) {}
		void visit(action_matcher& m) {}
		void visit(opt_matcher& m) {}
		void visit(many_matcher& m) {}
		void visit(some_matcher& m) {}
		void visit(seq_matcher& m) {}
		void visit(alt_matcher& m) {}
		void visit(look_matcher& m) {}
		void visit(not_matcher& m) {}
		void visit(capt_matcher& m) {}
	}; /* class default_visitor */

	/** Represents a grammar rule.
	 *  Pairs a name and optional type with a matching rule. The contained rule 
	 *  will be deleted on destruction. */
	class grammar_rule {
	public:
		grammar_rule(string name) : name(name), type("") {}
		grammar_rule(string name, shared_ptr<matcher> m) : name(name), type(""), m(m) {}
		grammar_rule(string name, string type, shared_ptr<matcher> m)
			: name(name), type(type), m(m) {}
		grammar_rule() {}
		
		string name;	/**< Name of the grammar rule */
		string type;	/**< Type of the grammar rule's return (empty for none) */
		shared_ptr<matcher> m;	/**< Grammar matching rule */
	}; /* class grammar_rule */
	typedef shared_ptr<grammar_rule> grammar_rule_ptr;

	/** Represents a Leg grammar. 
	 *  Deletes the contained grammar rules on destruction. */
	class grammar {
	public:
		grammar() {}

		grammar& operator += (shared_ptr<grammar_rule> r) {
			rs.push_back(r);
			names.insert(std::make_pair(r->name, r));
			return *this;
		}

		vector<shared_ptr<grammar_rule>> rs;	/**< list of grammar rules */
		unordered_map<string, shared_ptr<grammar_rule>> names;
										/**< lookup table of grammar rules by name */
		string pre, post;				/**< pre and post-actions */
	}; /* class grammar */
	typedef shared_ptr<grammar> grammar_ptr;
	
} /* namespace ast */

