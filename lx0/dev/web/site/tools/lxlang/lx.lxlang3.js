try { eval("lx"); } catch (e) { lx = {}; }
lx.lxlang3 = {};

(function(NS) {
    

    function _error (s)
    {
        for (var i = 1; i < arguments.length; ++i)
        {
            var regex = new RegExp("%" + i + "%", "g");
            s = s.replace(regex, arguments[i]);
        }
        console.log(s);
        throw s;
    }

    /*
        Convert the text string into a token array.
     */
    NS.tokenize = function (text)
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
    };

    /*
        Conditionally advance along a token array based on basic pattern matching.
     */
    NS.Lexer = (function() {

        var ctor = function(tokens)
        {
            this._index = 0;
            this._tokens = tokens;
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
                    var match = [];

                    for (var i = 0; i < arguments.length; ++i)
                    {
                        var pattern = arguments[i];
                        var next = this.peek(i);

                        if (pattern.substr(0,1) == "@") {
                            if (next.type == pattern.substr(1))
                                match.push(next);
                        }
                        else {
                            if (next.value == pattern)
                                match.push(next);
                        }
                    }

                    // At least one requested match failed, so fail completely
                    if (match.length != arguments.length)
                        match = undefined;
                }

                if (match)
                    this._index += match.length;

                return match;    
            },

            peek : function(offset) 
            {
                offset = offset || 0;
                var next = this._tokens[this._index + offset];
                return next ? next : {};
            },
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];

        return ctor;
    })();

    NS.ParserStage1 = (function() {

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

            'statement' : function (lex)
            {
                if (lex.empty()) _error("E1016 Cannot parse empty statement");

                var tokens = [];
                while (!lex.empty() && lex.peek().value != ';')
                {
                    tokens.push(lex.consume()[0]);
                }
                lex.consume(';') || _error("E1002: Did not find expected ;");
                
                return { type : 'statement', tokens : tokens };
            },

            'statementSet' : function (lex)
            {
                if (lex.empty()) _error("E1016 Cannot parse empty statementSet");
                
                var statements = [];

                while (!lex.empty() && lex.peek().value != '}')
                {
                    var statement = this.parse('statement') || _error("Could not parse Statement");
                    statements.push(statement);
                }
                return { type : 'statementSet', statements : statements };
            },

            'variableDeclaration' : function(lex)
            {
                var dec = lex.consume("@name", "@name") || _error("Expected variable declaration");
                return { type : "variableDeclaration", type : dec[0].value, name : dec[1].value };
            },

            'argumentList' : function (lex)
            {
                var ast = { type : "argumentList", list : [] };

                var dec = this.parse('variableDeclaration');
                while (lex.consume(","))
                {
                    ast.list.push(dec);
                    dec = this.parse('variableDeclaration');
                }
                ast.list.push(dec);
                
                return ast;
            },

            'function' : function (lex)
            {
                var ast = { type : 'function' };
                
                var header1 = lex.consume("function", "@name", "(");
                ast.argumentList = this.parse('argumentList');
                var header2 = lex.consume(")", "->", "@name");
                
                ast.name = header1[1].value;
                ast.returnType = header2[2].value;

                lex.consume("{");
                while (lex.peek().value != "}")
                {
                    ast.body = this.parse('statementSet');
                }
                return ast;
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

                var parseFunction = parsers[type] || _error("No parse function for %1%", JSON.stringify(type));
                var ast = parseFunction.call(this, this._lex, options);
                var endPos = this._lex.tell();
                    
                return ast;
            },
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];

        return ctor;
    })();

    NS.ParserStage2 = (function() {
        
        var transforms =
        {
            'statement' : function(ast)
            {
                var lex = new NS.Lexer(ast.tokens);
                var parser = new NS.ParserStage1(lex);
                
                var next = lex.peek();
                
                if (next.value == "var")
                {
                    lex.consume("var");
                    var name = lex.consume("@name")[0].value;
                    lex.consume("=");
                    var expr = parser.parse('expression');
                    lex.consume(";");
                    return { type : 'assignment', variable: name, value : expr };                    
                }   
                else
                {
                    var expr = parser.parse('expression');
                    lex.consume(";");
                    return { type : 'statement', value : expr };
                }
            },

            'statementSet' : function (ast)
            {
                for (var i = 0; i < ast.statements.length; ++i)
                    ast.statements[i] = this.parse(ast.statements[i]);
                return ast;
            },

            'function' : function (ast)
            {
                ast.body = this.parse(ast.body);
                return ast;
            }
        };

        var methods =
        {
            parse : function (ast, options)
            {
                var type = ast.type;
                var transform = transforms[type];
                return (transform) ? transform.call(this, ast, options) : ast;
            }
        };
        
        var ctor = function() 
        {
        };

        for (var name in methods)
            ctor.prototype[name] = methods[name];
        return ctor;
    })();

    NS.Parser2 = (function() {

        var methods = 
        {
            _parseStage1 : function(lexer, nonterminal)
            {
                var parser = new NS.ParserStage1(lexer);
                return parser.parse(nonterminal);
            },

            _parseStage2 : function(ast)
            {
                var parser = new NS.ParserStage2();
                return parser.parse(ast);
            },

            parse : function(text, nonterminal)
            {
                var tokens = NS.tokenize(text);
                var lexer = new NS.Lexer(tokens);
                
                var ast1 = this._parseStage1(lexer, nonterminal);
                var ast2 = this._parseStage2(ast1);

                return ast2;
            }
        };
        
        var ctor = function()
        {
        };
        
        for (var name in methods)
            ctor.prototype[name] = methods[name];

        return ctor;
    })();

    NS.parse = function (text, nonterminal)
    {
        nonterminal = nonterminal || "translationUnit";
        
        var parser = new NS.Parser2();
        return parser.parse(text, nonterminal);
    }

})(lx.lxlang3);
