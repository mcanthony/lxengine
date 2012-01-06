try { eval("lx"); } catch (e) { lx = {}; }
lx.lxlang3 = {};

(function(NS) {
    
    NS.Lexer = (function() {

        function _parseTokens(text)
        {
            var Terminals  =
            {
                "whitespace" : 
                { 
                    re : /^[\s\n]+/, 
                    discard : true 
                },
                "name" : 
                { 
                    re : /^[A-Za-z\$_][\$A-Za-z0-9_]*/ 
                }, 
                "number" : 
                {
                    reArr : [
                        /^[0-9]+(\.[0-9]+)?/,
                        /^\.[0-9]+/
                    ],
                },
                "symbol" : 
                {
                    arr : [ 
                        "+", "-", "*", "/",
                        "%",
                        "=",
                        "==", "<", ">", "<=", ">=",
                        "(", ")",
                        "{", "}",
                        "[", "]",
                        ";", ",",
                        "?", ":",
                        "->", "."
                    ],
                },
                "comment" :
                {
                    re : /^\/\/[^\n]*\n/,
                    discard : true,
                },
                "unrecognized" :
                {
                },
            };

            var tokens = [];
            
            while (text.length)
            {
                //
                // Convenience function to set a new match if it is longer than the
                // current match or there has not yet been a match
                // 
                var match = undefined;
                function setMatch (type, terminal, text)
                {
                    if (!match || text.length > match.text.length)
                    {
                        match = {
                            type : type,
                            terminal : terminal,
                            text : text
                        };
                    }
                }

                //
                // Loop through all the tests in the terminal table
                //
                for (var key in Terminals) {
                    var terminal = Terminals[key];

                    if (terminal.re)
                    {
                        var m;
                        if (m = terminal.re.exec(text)) {
                            setMatch(key, terminal, m[0]);
                        }
                    }
                    if (terminal.reArr)
                    {
                        for (var i = 0; i < terminal.reArr.length; ++i)
                        {
                            if (m = terminal.reArr[i].exec(text)) {
                                setMatch(key, terminal, m[0]);
                            }
                        }
                    }
                    if (terminal.arr)
                    {
                        for (var i = 0; i < terminal.arr.length; ++i)
                        {
                            var s = terminal.arr[i];
                            if (text.substr(0, s.length) == s)
                                setMatch(key, terminal, s);
                        }
                    }
                }

                //
                // Set a single character match if no rules were matched
                //
                setMatch("unrecognized", Terminals["unrecognized"], text.substr(0,1));

                //
                // Advanced the text stream and push the token into the stream
                //
                text = text.substr(match.text.length);
                if (!match.terminal.discard)
                {
                    tokens.push({ type : match.type, value : match.text });
                }
            }
            return tokens;
        }

        var ctor = function(arg)
        {
            this._index = 0;

            if (typeof arg == "string")
                this._tokens = _parseTokens(arg);
            else
                this._tokens = arg;
        };

        var methods = 
        {
            tell  : function () 
            { 
                return this._index; 
            },
            
            seek : function(index) 
            { 
                this._index = index; 
            },
            
            empty : function() 
            {
                return (this._index == this._tokens.length);
            },

            consume : function() 
            {
                var next = this.peek();
                
                if (arguments.length == 0)
                {
                    match = [ next ];
                }
                else
                {
                    var pattern = arguments[0];

                    var match;
                    if (pattern.substr(0,1) == "@") {
                        if (next.type == pattern.substr(1))
                            match = [ next ];
                    }
                    else {
                        if (next.value == pattern)
                            match = [ next ];
                    }
                }

                if (match)
                    this._index++;

                return match;    
            },

            peek : function() 
            {
                return this._tokens[this._index];
            },
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];

        return ctor;
    })();

    NS.Parser = (function() {

        var ctor = function(lex)
        {
            this._lex = lex;
            this._context = {};
        };

        var parsers =
        {
            'name' : function (lex)
            {
                var term = lex.consume("@name");
                if (term)
                    return { type : 'name', value : term[0].value };
            },

            'number' : function (lex)
            {
                var terminal = lex.consume("@number");
                if (terminal)
                    return { type: 'number', value : parseFloat(terminal[0].value) };
            },


            'value' : function(lex)
            {
                var value = this.parse('number')
                    || this.parse('name');
                return value;
            },

            'function' : function (lex)
            {
            },

            'statement' : function (lex)
            {
                var statements = [];

                var tokens = [];
                while (lex.peek() && lex.peek().value != ';')
                {
                    tokens.push(lex.consume()[0]);
                }
                lex.consume(';') || this.error("EP1002", "Did not find expected ;");
                
                return { type : 'statement', tokens : tokens };
            },

            /*
                EXPR 
                    = '(' EXPR ')' 
                    | VALUE
                    | EXPR + EXPR
                    | EXPR - EXPR
                    | EXPR * EXPR
                    | EXPR / EXPR

                Algorithm:
                    parse all non-left recursive first
                    then peek next symbol to decide if to recurse
                        if next symbol higher precedence than current, do the parse
                        else return
             */
            'expression' : function(lex)
            {
                var operatorPrecedence =
                {
                    "*" : 10,
                    "/" : 10,
                    "%" : 10,
                    "+" : 8,
                    "-" : 8,
                };

                function parseNR (lex)
                {
                    if (lex.consume('('))
                    {
                        var expr = this.parse('expression');
                        lex.consume(')') || this.error("EP1001", "Expected ')'");
                        return { type : 'parenthetical', value : expr };
                    }
                    else 
                        return this.parse('value');
                }

                function parseR(lex)
                {
                    var next;
                    if (next = lex.peek())
                    {
                        switch (next.value)
                        {
                            case '+': 
                            case '-':
                            case '*':
                            case '/':
                                lex.consume(next.value);
                                return { type : 'infix', operator : next.value, rvalue : this.parse('expression'), precedence : operatorPrecedence[next.value] };
                        }
                    }
                }

                var lvalue = parseNR.call(this, lex);
                
                var rvalue;
                while (rvalue = parseR.call(this, lex))
                {
                    rvalue.lvalue = lvalue;
                    lvalue = rvalue;

                    // Shift the AST to account for precendence
                    if (lvalue.rvalue.precedence < lvalue.precedence)
                    {
                        var parent = lvalue.rvalue;
                        lvalue.rvalue = lvalue.rvalue.lvalue;
                        parent.lvalue = lvalue;
                        lvalue = parent;
                    }
                 }
                
                return lvalue;
            }
        };

        var methods = 
        {
            error : function (code, msg)
            {
                throw [code, msg];
            },

            parse : function(type, options)
            {
                var startPos = this._lex.tell();

                var ast = parsers[type].call(this, this._lex, options);
                var endPos = this._lex.tell();
                    
                return ast;
            }
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];

        return ctor;
    })();

})(lx.lxlang3);
