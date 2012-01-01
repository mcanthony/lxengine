
var lxlang2 = (function() {
      
    var NS = {};


        function _printformatted ()
        {
            var s = arguments[0];

            for (var j = 1; j < arguments.length; ++j) {
                var pattern = "%" + j + "%";
                var value = "" + arguments[j];
            
                var i = s.indexOf(pattern);
                while (i != -1)
                {
                    s = s.substr(0, i) + value + s.substr(i + pattern.length);
                    i = s.indexOf(pattern, i + value.length);
                }           
            }
            return s;
        };

    //
    // The lexical analyzer.  A glorified regular expression matcher that advances along a source
    // string as matches are made.
    //
    NS.Lexer = (function() {

        var ctor = function(text) {
            this._source = text;
            this._text = text;
            this._location = 0;
        };

        var methods = {

            //
            // Table of terminal symbols, as defined by regular expressions
            //
            _RETable : 
            {
                ws : /^\s+/,
                comment : /^\/\/[^\n]*\n\s*/,
                
                number : /^[0-9\.][0-9]*/,
                
                operator : /^[\+\*\-\/\%\<\>]|\|\|/,
                
                name        : /^[A-Za-z_][A-Za-z_0-9]*/,
                complexName : /^[A-Za-z_][A-Za-z_0-9\.]*/,
            },

            //
            //
            _skipComments : function()
            {
                var re = this._RETable["comment"];
                var m = this._text.match(re);
                while (m) 
                {
                    this._text = this._text.substr(m[0].length);
                    this._location += m[0].length;
                    m = this._text.match(re);
                }
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
                    this._skipComments();

                    var pattern = arguments[i];
                    var regExpr = (pattern.substr(0,1) == "@");
                    var optional = (pattern.length > 1 && pattern.substr(pattern.length - 1, 1) == "?");
                    
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
                    {
                        this._text = this._text.substr(match.length);
                        this._location += match.length;
                    }

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
                var prevloc = this._location;

                var matches = this.consume.apply(this, arguments);
                
                this._text = previous;
                this._location = prevloc;

                return matches;
            },

            tell : function()
            {
                return [ this._text, this._location ];
            },

            seek : function (s)
            {
                this._text = s[0];
                this._location = s[1];
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

            this._symbolStack = [{}];
        };

        var methods = {

            _pushSymbols : function() 
            {
                this._symbolStack.push({});
            },
            _popSymbols : function()
            {
                this._symbolStack.pop();
            },
            _symbolInfo : function(symbol)
            {
                var info;
                for (var i = this._symbolStack.length - 1; !info && i >= 0; --i)
                    info = this._symbolStack[i][symbol];
                return info;
            },
            _addSymbol : function(symbol, info)
            {
                this._symbolStack[this._symbolStack.length - 1][symbol] = info;
            },
            _eraseSymbol : function(symbol)
            {
                delete this._symbolStack[this._symbolStack.length - 1];
            },


            _error : function(lex, msg)
            {
                var args = Array.prototype.slice.call(arguments, 1);
                var s = _printformatted.apply(null, args);
                
                msg = "ERROR: " + s + ", around:\n---> '" + lex._text.substr(0, 80) + "'";
                throw msg;
            },

            _warn : function(lex, msg)
            {
                var args = Array.prototype.slice.call(arguments, 1);
                var s = _printformatted.apply(null, args);
                console.log("WARNING: ", s);
            },

            parseNumber : function(lex)
            {
                var number = lex.consume("@number");
                if (number) 
                    return { type: "number", value: parseFloat(number[0]), rttype : "float" };
            },

            parseName : function(lex)
            {
                var name = lex.consume("@name");
                if (name) {
                    var info = this._symbolInfo(name[0]);
                    if (!info)
                    {
                        console.log("Symbol stack:", this._symbolStack);                        
                        this._error(lex, "E1001 No symbol info for variable named '%1%'", name[0]);                        
                    }
                    if (!info.rttype)
                    {
                        console.log("Symbol stack:", this._symbolStack);
                        this._error(lex, "E1002 No type info for variable named '%1%'", name[0]);
                    }

                    return { type : "name", value: name[0], rttype : info.rttype };
                }
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
                        return { type : "member", variable : name, index : v, rttype : "float" };
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
                '%' : 3,
                '<' : 2,
                '>' : 2,
                '||' : 1,
            },          

            parseOperator : function(lex)
            {
                var op = lex.consume("@operator");
                var precedence = this._operatorPrecedence[op];
                if (op) 
                    return { type : "operator", value : op[0], precedence : precedence }
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
                        if (second) {
                            first.parentType = "infix";
                            second.parentType = "infix";

                            var rttype;
                            if (first.rttype == second.rttype)
                            {
                                rttype = first.rttype;
                            }
                            else
                            {
                                var symbol = operator.value + "$" + first.rttype + "$" + second.rttype;
                                var info = this._symbolInfo(symbol);
                            
                                if (!info || !info.rttype)
                                {
                                    console.log("Symbol stack:", this._symbolStack);
                                    this._error(lex, "No symbol information for '%1%'", symbol);
                                }
                                rttype = info.rttype;
                            }

                            return { 
                                type : "infix", 
                                operator : operator.value, 
                                valueA : first, 
                                valueB : second, 
                                rttype : rttype 
                            };
                        }
                    }
                }

                var choice = lex.peek("@ws?", "?", "@ws?");
                if (choice)
                {
                    choice = this._parse(lex, 'choiceExpression');
                    if (choice) return {
                        type : 'choice',
                        testExpr : first,
                        valueA : choice.valueA,
                        valueB : choice.valueB,
                    };
                }

                lex.seek(s0);
            },

            parseInfixExpression : function(lex, precedence)
            {
                precedence = precedence || 0;

                var s = lex.tell();
                lex.consume("@ws?");
                var first = this._parse(lex, 'functionCall') || this._parse(lex, 'parenthetical') || this.parseValue(lex);
                
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

            

            parseExpression : function(precedence, lex)
            {
                return this.parseInfixExpression(lex, 0);
            },

            _parsers : 
            {
                translationUnit : function(lex)
                {
                    var m = this._parse(lex, 'module');
                    if (!m)
                    {
                        this._error(lex, "Module definition not found in translation unit.");
                        return;
                    }

                    var funcs = [];
                    var f = this._parse(lex, 'function');
                    while (f)
                    {
                        funcs.push(f);
                        f = this._parse(lex, 'function');
                    }
                    lex.consume("@ws?");

                    if (!lex.empty())
                        this._error(lex, "Expected end of translationUnit");

                    if (m && lex.empty())
                        return { type : "translationUnit", module : m, functions : funcs };
                },

                parenthetical : function(lex)
                {
                    if (!lex.consume("@ws?", "(", "@ws?")) return;
                    var expr = this.parseExpression(0, lex);
                    var end = lex.consume("@ws?", ")", "@ws?");

                    if (expr && end)
                        return {
                            type : "parenthetical",
                            value : expr
                        };
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
                    var b = lex.consume("@ws?", ")", "@ws?");
                    if (r && b)
                    {
                        var fullName = [ r[1] ];
                        for (var i in args)
                            fullName.push(args[i].rttype);
                        fullName = fullName.join("$");

                        var info = this._symbolInfo(fullName);
                        if (!info)
                        {
                            console.log("Symbol stack:", this._symbolStack);
                            this._warn(lex, "E1003 No symbol info for '%1%'.  Expected function name.", fullName);
                        }
                        if (!info.rttype)
                        {
                            console.log("Symbol stack:", this._symbolStack);
                            this._warn(lex, "E1004 No return type info for '%1%'", fullName);
                        }

                        return { type : "functionCall", value : r[1], fullName: fullName, args: args, rttype : info.rttype };
                    }
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
                    var retVal;

                    var name;
                    var pushed = false;
                    
                    do
                    {
                        lex.consume("@ws?");

                        var startLocation = lex._location;

                        var funcdecl = lex.consume("function");
                        if (!funcdecl) break;

                        var hdr = lex.consume("@ws", "@name", "@ws?", "(");
                        if (!hdr) 
                        {
                            this._error(lex, "Error parsing function");
                            break;
                        }
                        name = hdr[1];

                        this._addSymbol(name, { rttype : "function" });
                        pushed = true;
                        this._pushSymbols();

                        var argList = this._parse(lex, 'argumentList');
                        if (!argList) 
                        {
                            this._error(lex, "Error parsing function");
                            break;
                        }

                        for (var i in argList.list)
                        {
                            var arg = argList.list[i];
                            this._addSymbol(arg.name, { rttype : arg.type } );
                        }

                        var retList = lex.consume("@ws?", ")", "@ws?", "->", "@ws?", "@name", "@ws?", "{");
                        if (!retList)
                        {
                            this._error(lex, "Error parsing function");
                            break;
                        }

                        var body = this._parse(lex, 'statementSet');
                        if (!body)
                        {
                            this._error(lex, "Error parsing function body");
                            break;
                        }

                        var end = lex.consume("@ws?", "}");
                        if (!end)
                        {
                            this._error(lex, "Error parsing function close");
                            break;
                        }

                        var endLocation = lex._location;

                        var source = lex._source.substr(startLocation, endLocation - startLocation);

                        retVal = { 
                            type : "function", 
                            name : name, 
                            argumentList : argList.list, 
                            returnType: retList[5], 
                            body : body,
                            source : source,
                        };
                    
                    } while (false);

                    if (pushed)
                        this._popSymbols();
                    if (!retVal)
                        this._eraseSymbol(name);
                    
                    return retVal;
                },

                returnExpression : function(lex)
                {
                    if (!lex.consume("@ws?", "return", "@ws?")) return;
                    var value = this.parseExpression(0, lex);
                    if (!value)
                    {
                        this._error(lex, "Expected expression");
                    }

                    if (!lex.consume("@ws?", ";", "@ws?"))
                    {
                        this._error(lex, "Expected ';'");
                    }

                    if (value)
                        return { type: "returnExpression", value : value };
                },

                choiceExpression : function(lex)
                {
                    if (!lex.consume("@ws?", "?", "@ws?")) return;

                    var left = this.parseExpression(0, lex);

                    if (!lex.consume("@ws?", ":", "@ws?")) return;

                    var right = this.parseExpression(0, lex);

                    if (left && right)
                        return { type: "choiceExpression", valueA : left, valueB : right };
                },

                assignment : function(lex)
                {
                    var start = lex.tell()[0].substr(0, 40) + "\n";

                    var lvalue = lex.consume("@ws?", "@name", "@ws", "@name", "@ws", "=");
                    if (!lvalue)
                        return;
                    var name = lvalue[3];
                    var type = lvalue[1];

                    this._addSymbol(name, { rttype : type });

                    var expr = this.parseExpression(0, lex);
                    var close = lex.consume("@ws?", ";", "@ws?");
                    
                    if (lvalue && expr)
                    {
                        if (!close)
                        {
                            this._error(lex, "Expected ';' to end statement. Around '%1%'", start);
                        }

                        return { type : "assignment", rttype : type, name : name, value : expr };
                    }
                },

                statement : function(lex)
                {
                    if (lex.peek("@ws?", "@name", "@ws", "@name", "@ws?", "="))
                        return this._parse(lex, "assignment");
                    else if (lex.peek("@ws?", "return"))
                        return this._parse(lex, "returnExpression");

                },

                statementSet : function (lex)
                {
                    var set = [];
                    var statement = this._parse(lex, 'statement');
                    while (statement)
                    {
                        set.push(statement);
                        statement = this._parse(lex, 'statement');
                    }
                    return { type : 'statementSet', value : set };
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
            parse : function(text, nonterminal)
            {
                nonterminal = nonterminal || 'translationUnit';

                var lexer = new NS.Lexer(text);

                this._pushSymbols();
                function add1to1 (type, namelist) {
                    var names = namelist.split(',');
                    for (var i in names) {
                        var name = names[i];
                        this._addSymbol(name + "$" + type, { rttype : type });
                    }
                }
                var standardFunctions = "abs,floor,fract,sqrt";
                add1to1.call(this, "float", standardFunctions);
                add1to1.call(this, "vec2", standardFunctions);
                add1to1.call(this, "vec3", standardFunctions);

                this._addSymbol("*$vec2$float", { rttype : "vec2" });
                this._addSymbol("dot$vec3$vec3", { rttype : "float" });
                this._addSymbol("length$vec2", { rttype : "float" });
                this._addSymbol("lengthSqrd$vec2", { rttype : "float" });
                this._addSymbol("vec2$float$float", { rttype : "vec2" });
                this._addSymbol("vec3$float$float$float", { rttype : "vec3" });
                this._addSymbol("vec4$float$float$float$float", { rttype : "vec4" });

                var result = this._parse(lexer, nonterminal);

                this._popSymbols();

                if (result)
                    this._decorate(result);

                return result;
            },

            _decorate : function(ast)
            {
                ast.functionNameTable = {};
                for (var i in ast.functions)
                    ast.functionNameTable[ast.functions[i].name] = ast.functions[i]; 
            },

        };

        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        }
        return ctor;
    })();

    NS.GenerateBase = (function() {
        var ctor = function() {
            this._body = "";
            this._indent = 0;
        };
        var methods =
        {
            incIndent : function()
            {
                this._indent += 4;
                this._body = this._body.replace(/\ +$/, this.indentWs());
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

            append : function ()
            {
                var s = _printformatted.apply(this, arguments);
                this._body += s.replace(/\n/g, "\n" + this.indentWs());                
            },

            resolveFunctionName : function(name)
            {
                return name;
            },

            _translators :
            {
                name : function (ast)
                {
                    this.append(ast.value);
                },

                number : function (ast)
                {
                    this.append("%1%", ast.value);
                },

                member : function (ast)
                {
                    this.append(ast.variable.value);
                    this.append("[" + ast.index.value + "]");
                },

                infix : function (ast)
                {
                    this._translate(ast.valueA);
                    this.append(" " + ast.operator + " ");
                    this._translate(ast.valueB);
                },

                

                functionCall : function (ast)
                {
                    var functionName = this.resolveFunctionName(ast.value, ast.fullName);

                    this.append(functionName + "(");
                    for (var i = 0; i < ast.args.length; ++i)
                    {
                        this._translate(ast.args[i]);
                        if (i + 1 < ast.args.length)
                            this.append(", ");
                    }
                    this.append(")");
                },

                choice : function(ast)
                {
                    this._translate(ast.testExpr);
                    this.append(" ? ");
                    this._translate(ast.valueA);
                    this.append(" : ");
                    this._translate(ast.valueB);
                },

                parenthetical : function (ast)
                {
                    this.append("(");
                    this._translate(ast.value);
                    this.append(")");
                },

                returnExpression : function (ast)
                {
                    this.append("return ");
                    this._translate(ast.value);
                },

                assignment : function(ast)
                {
                    this.append("%1% %2% = ", "var", ast.name);
                    this._translate(ast.value);
                },

                statementSet : function(ast)
                {
                    for (var i = 0; i < ast.value.length; ++i)
                    {
                        this._translate(ast.value[i]);
                        this.append(";\n");
                    }
                },
            },

            _translate : function (ast)
            {
                var tr = this._translators[ast.type];
                if (!tr)
                {
                    console.log("No translator available for '" + ast.type + "'");
                    this.append("UNKNOWN_AST_" + ast.type);
                }
                else
                    tr.call(this, ast);
            },
            
            translate : function (ast) 
            {
                this._astRoot = ast;
                this._translate(ast);
                return this._body;
            },                      
        };

        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        };
        return ctor;
    })();

    NS.GenerateJavascript = (function() {
        var ctor = function() {
            NS.GenerateBase.call(this);
        };

        var methods = {


            _predefinedFunctions :
            {
                "sqrt" : "Math.sqrt",
                "floor" : "Math.floor",
                "abs" : "Math.abs",
                "vec2" : "new Array",
                "vec3" : "new Array",
            },

            _predefinedFunctions2 :
            {
                "abs$vec2" : "_lxbb_abs_vec2",
                "floor$vec2" : "_lxbb_floor_vec2",
                "fract$vec2" : "_lxbb_fract_vec2",
            },

            resolveFunctionName : function(name, type)
            {
                if (this._astRoot.functionNameTable[name])
                    return  "NS." + name;
                
                if (type)
                {
                    var predefined2 = this._predefinedFunctions2[type];
                    if (predefined2)
                        return predefined2;
                }

                var predefined = this._predefinedFunctions[name];
                if (predefined)
                    return predefined;

                name = "_lxbb_" +  type.replace(/\$/g, "_");

                return name;
            },

            _translators : 
            {

                _op_name :
                {
                    "*" : "mul",
                    "/" : "div",
                    "+" : "add",
                    "-" : "sub",
                    "%" : "mod",
                    "<" : "lt",
                    ">" : "gt",
                },

                infix : function(ast)
                {
                    if (ast.rttype == "float")
                    {
                        this._translate(ast.valueA);
                        this.append(" %1% ", ast.operator);
                        this._translate(ast.valueB);
                    }
                    else
                    {
                        var rttype = (ast.valueA.rttype == ast.valueB.rttype) ? ast.valueA.rttype : ast.valueA.rttype + "_" + ast.valueB.rttype;
                        this.append("_lxbb_%1%_%2%(", this._translators._op_name[ast.operator], rttype);
                        this._translate(ast.valueA);
                        this.append(", ");
                        this._translate(ast.valueB);
                        this.append(")");
                    }
                },

                "function" : function (ast)
                {
                    if (this._astRoot.type != "function")
                        this.append("NS.%1% = ", ast.name);

                    this.append("function(");
                    for (var i = 0; i < ast.argumentList.length; ++i)
                    {
                        var arg = ast.argumentList[i];
                        this.append(arg.name );

                        if (i + 1 < ast.argumentList.length)
                            this.append( ", " );
                        else
                            this.append( ")\n" );
                    }
                    this.append( "{\n" );
                    this.incIndent();
                    this._translate(ast.body);
                    this.decIndent();
                    this.append( "}");
                    
                    if (this._astRoot.type != "function")
                    {
                        this.append( ";\n" );
                        
                        this.append("NS.%1%.source = %2%;\n\n", ast.name, JSON.stringify(ast.source));
                    }
                },

                translationUnit : function(ast)
                {
                    var module = ast.module.value.split(".");

                    this.append("// Ensure the namespace exists\n");
                    this.append("try { eval(\"%1%\"); } catch (e) { %1% = {}; };\n", module[0]);

                    for (var i = 1; i < module.length; ++i) {
                        this.append("if (!");
                        for (var j = 0; j < i; ++j)
                            this.append("%1%.", module[j]);
                        this.append("%1%", module[i]);
                        this.append(") ");

                        for (var j = 0; j < i; ++j)
                            this.append("%1%.", module[j]);                        
                        this.append("%1% = {};\n", module[i]);
                    } 
                    this.append("\n");

                    this.append("(function(NS) {\n\n");                   
                    this.incIndent();                  
                    for (var i in ast.functions)                
                        this._translate(ast.functions[i]);
                    this.decIndent();
                    this.append("})(%1%);\n", ast.module.value);

                },
            }
        };

        for (var key in NS.GenerateBase.prototype) {
            ctor.prototype[key] = NS.GenerateBase.prototype[key];
        }
        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        }

        for (var key in NS.GenerateBase.prototype._translators) {
            if (!ctor.prototype._translators[key])
                ctor.prototype._translators[key] = NS.GenerateBase.prototype._translators[key];
        }
        
        return ctor;
    })();

    NS.GenerateCpp = (function() {
        var ctor = function() {
            NS.GenerateBase.call(this);
        };

        var _typeMap = {
            vec3 : "glm::vec3",
            "var" : "float",
        };

        var methods =
        {
            _translators :
            {            
                "function" : function (ast)
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
                    this.incIndent();
                    this._translate(ast.body);
                    this.decIndent();
                    this.append( "}\n" );
                    this.append( "\n" );
                },

                translationUnit : function(ast)
                {
                    var module = ast.module.value.split(".");
                    for (var i in module) {
                        this._body += "namespace " + module[i] + " { ";
                    } 

                    this._body += "\n";  
                    this._body += "\n";

                    this.incIndent();
                    for (var i in ast.functions)                
                        this._translate(ast.functions[i]);
                    this.decIndent();

                    for (var i in module)
                        this._body += "}";
                    this._body += "\n";
                },
            },
        };

        for (var key in NS.GenerateBase.prototype) {
            ctor.prototype[key] = NS.GenerateBase.prototype[key];
        }
        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        };
        for (var key in NS.GenerateBase.prototype._translators) {
            ctor.prototype._translators[key] = NS.GenerateBase.prototype._translators[key];
        }
        for (var key in methods._translators) {
            ctor.prototype._translators[key] = methods._translators[key];
        }

        return ctor;
    })();


    return NS;
})();

