
var lxlang2 = (function() {
      
    var NS = {};

    //
    // The lexical analyzer.  A glorified regular expression matcher that advances along a source
    // string as matches are made.
    //
    NS.Lexer = (function() {

        var ctor = function(text) {
            this._text = text;
        };

        var methods = {

            //
            // Table of terminal symbols, as defined by regular expressions
            //
            _RETable : 
            {
                ws : /^\s+/,
                
                number : /^[0-9][0-9]*/,
                
                operator : /^\+|\-|\*|\//,
                
                name        : /^[A-Za-z_][A-Za-z_0-9]*/,
                complexName : /^[A-Za-z_][A-Za-z_0-9\.]*/,
            },

            //
            // Takes an array of arguments, each specifying a terminal to consume.
            //
            // If the terminal name begins with "@", the name will be looked up in the 
            // regular expression table.
            //
            // If the terminal name ends with "?", the terminal is considered optional
            // and will be consumed if it is there.
            //
            // The return value is an array of the matched terminals literal values;
            // in the case of optional terminals, if they are not present, the value
            // for that terminal in the array will be undefined.
            //
            // If any non-optional terminals were not matched, the return value will be
            // undefined.
            //
            consume : function()
            {
                var matches = [];
                var valid = true;

                for (var i = 0; i < arguments.length;++i)
                {
                    var pattern = arguments[i];
                    var regExpr = (pattern.substr(0,1) == "@");
                    var optional = (pattern.substr(pattern.length - 1, 1) == "?");
                    
                    if (regExpr) pattern = pattern.substr(1);
                    if (optional) pattern = pattern.substr(0, pattern.length - 1);

                    var match = undefined;
                    if (regExpr)
                    {
                        var re = this._RETable[pattern];
                        var m = this._text.match(re);
                        if (m) 
                            match = m[0];
                    }
                    else
                    {
                       if (this._text.substr(0, pattern.length) == pattern)
                            match = pattern;
                    }

                    // Consume the element by advancing in the text string
                    if (match)
                        this._text = this._text.substr(match.length);

                    if (!optional && !match)
                        valid = false;

                    matches.push(match);
                }
                return valid ? matches : undefined;
            },

            peek : function() 
            {
                // Saving the full string is likely inefficient for large bodies of text.
                var previous = this._text;
                var matches = this.consume.apply(this, arguments);
                this._text = previous;
            },

            tell : function()
            {
                return this._text;
            },

            seek : function (s)
            {
                this._text = s;
            },

            empty : function()
            {
                return this._text.length == 0;
            },

        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];
        return ctor;
    })();

    //
    // The grammar parser that builds an AST (Abstract Syntax Tree) for later conversion to
    // a target language.
    //
    NS.Parser = (function() {
        var ctor = function() {
        };

        var methods = {

            _error : function(lex, msg)
            {
                msg = "Parse error: '" + msg + "' around '" + lex._text.substr(0, 20) + "'";
                throw msg;
            },

            parseNumber : function(lex)
            {
                var number = lex.consume("@number");
                if (number) 
                    return { type: "number", value: parseFloat(number[0]) };
            },

            parseName : function(lex)
            {
                var name = lex.consume("@name");
                if (name) 
                    return { type : "name", value: name[0] };
            },

            parseMember : function(lex)
            {
                var s0 = lex.tell();
                var name = this.parseName(lex);
                if (name)
                {
                    var r = lex.consume("@ws?", "[", "@ws?");
                    var v = this.parseValue(lex);
                    var t = lex.consume("@ws?", "]", "@ws?");
                    if (r && v && t)
                        return { type : "member", variable : name, index : v };
                }
                lex.seek(s0);
            },

            parseValue : function(lex)
            {
                return this.parseNumber(lex) 
                    || this.parseMember(lex)
                    || this.parseName(lex);
            },

            _operatorPrecedence :
            {
                '*' : 8,
                '/' : 8,
                '+' : 4,
                '-' : 4,
            },

            parseOperator : function(lex)
            {
                var op = lex.consume("@operator");
                var precedence = this._operatorPrecedence[op];

                if (op)
                    return { type : "operator", value : op[0], precedence : precedence };
            },

            parseExpressionR : function(first, precedence, lex)
            {
                var s0 = lex.tell();
                
                var operator = this.parseOperator(lex);
                if (operator)
                {
                    if (operator.precedence >= precedence) 
                    {
                        lex.consume("@ws?");
                        var second = this.parseExpression(operator.precedence, lex);
                        if (second)
                            return { type : "infix", operator : operator.value, valueA : first, valueB : second };
                    }
                }
                lex.seek(s0);
            },

            parseExpression : function(precedence, lex)
            {
                var s = lex.tell();
                lex.consume("@ws?");
                var first = this._parse(lex, 'functionCall') || this.parseValue(lex);
                
                if (first)
                {                
                    lex.consume("@ws?");
                    var second = this.parseExpressionR(first, precedence, lex);
                    while (second)
                    {
                        first = second;
                        second = this.parseExpressionR(first, precedence, lex);
                    }
                    return first;
                }
                lex.seek(s);
            },

            _parsers : 
            {
                translationUnit : function(lex)
                {
                    var m = this._parse(lex, 'module');
                    var funcs = [];

                    var f = this._parse(lex, 'function');
                    while (f)
                    {
                        console.log(JSON.stringify(f, null, "\t"));
                        funcs.push(f);
                        f = this._parse(lex, 'function');
                    }
                    lex.consume("@ws?");

                    if (!lex.empty())
                        this._error(lex, "Expected end of translationUnit");

                    if (m && lex.empty())
                        return { type : "translationUnit", module : m, functions : funcs };
                },


                module : function(lex)
                {
                    var r = lex.consume("@ws?", "module", "@ws", "@complexName", "@ws?", ";");
                    if (r)
                        return { type : "module", value : r[3] };
                },

                functionCall : function (lex)
                {
                    var r = lex.consume("@ws?", "@name", "@ws?", "(");
                    if (!r) return;

                    console.log("R:" , r);

                    var args = [];

                    var a = this.parseExpression(0, lex);
                    while (a)
                    {
                        args.push(a);
                        if (lex.consume("@ws?", ",", "@ws?"))
                            a = this.parseExpression(0, lex);
                        else
                            a = undefined;
                    }
                    console.log(args);
                    var b = lex.consume("@ws?", ")", "@ws?");
                    if (r && b)
                        return { type : "functionCall", value : r[1], args: args };
                },

                variableDeclaration : function(lex)
                {
                    var dec = lex.consume("@ws?", "@name", "@ws", "@name");
                    if (dec) 
                        return { type : "variableDeclaration", type : dec[1], name : dec[3] };
                },

                argumentList : function (lex)
                {
                    var list = [];
                    var dec = this._parse(lex, 'variableDeclaration');
                    while (lex.consume(","))
                    {
                        list.push(dec);
                        dec = this._parse(lex, 'variableDeclaration');
                    }
                    list.push(dec);
                    return { type : "argumentList", list : list };
                },

                "function" : function (lex)
                {
                    var hdr = lex.consume("@ws?", "function", "@ws", "@name", "@ws?", "(");
                    if (!hdr) return;


                    var argList = this._parse(lex, 'argumentList');
                    if (!argList) return;

                    var retList = lex.consume("@ws?", ")", "@ws?", "->", "@ws?", "@name", "@ws?", "{");
                    if (!retList) return;

                    var body = this._parse(lex, 'returnExpression');
                    if (!body) return;

                    var end = lex.consume("@ws?", "}");
                    if (!end) return;

                    return { type : "function", name : hdr[3], argumentList : argList.list, returnType: retList[5], body : body };
                },

                returnExpression : function(lex)
                {
                    if (!lex.consume("@ws?", "return", "@ws?")) return;
                    var value = this.parseExpression(0, lex);
                    lex.consume("@ws?", ";", "@ws?");
                    if (value)
                        return { type: "returnExpression", value : value };
                },
            },

            _parse : function (lex, nonterminal)
            {
                var save = lex.tell();
                var result = this._parsers[nonterminal].call(this, lex);
                if (result)
                    return result;
                lex.seek(save);
            },

            //
            // Entry point for the translation unit
            //
            parse : function(text)
            {
                var lexer = new NS.Lexer(text);
                var result = this._parse(lexer, 'translationUnit');
                return result;
            },

        };

        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        }
        return ctor;
    })();

    NS.GenerateCpp = (function() {
        var ctor = function() {
            this._body = "";
            this._indent = 0;
        };

        var _typeMap = {
            vec3 : "glm::vec3",
            "var" : "float",
        };

        var methods =
        {
            incIndent : function()
            {
                this._indent += 4;
                for (var i = 0; i < this._indent; ++i)
                    this._body += " ";
            },

            decIndent : function()
            {
                this._indent -= 4;
                this._body = this._body.replace(/\ +$/, this.indentWs());
            },

            indentWs : function()
            {
                var ws = "";
                for (var i = 0; i < this._indent; ++i)
                    ws += " ";
                return ws;
            },

            append : function (s)
            {
                var ws = this.indentWs();
                this._body += s.replace("\n", "\n" + ws);
            },

            translateFunction : function (ast)
            {
                this.append( _typeMap[ast.returnType] + " " + ast.name + " (" );
                for (var i = 0; i < ast.argumentList.length; ++i)
                {
                    var arg = ast.argumentList[i];
                    this.append( _typeMap[arg.type] + " " + arg.name );

                    if (i + 1 < ast.argumentList.length)
                        this.append( ", " );
                    else
                        this.append( ")\n" );
                }
                this.append( "{\n" );
                this.append( "}\n" );
                this.append( "\n" );
            },

            translate : function (ast) 
            {
                var module = ast.module.value.split(".");
                for (var i in module) {
                    this._body += "namespace " + module[i] + " { ";
                } 

                this._body += "\n";  
                this._body += "\n";

                this.incIndent();
                for (var i in ast.functions)                
                    this.translateFunction(ast.functions[i]);
                this.decIndent();

                for (var i in module)
                    this._body += "}";
                this._body += "\n";

                return this._body;
            },
        };

        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        };
        return ctor;
    })();


    return NS;
})();

