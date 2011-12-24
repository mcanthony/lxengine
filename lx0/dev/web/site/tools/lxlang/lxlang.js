
var lxlang2 = (function() {
      
    var NS = {};

    NS.Lexer = (function() {
        var ctor = function(text) {
            this._text = text;
        };

        var methods = {

            _RETable : 
            {
                ws : /^\s+/,
                number : /^[1-9][0-9]*/,
                operator : /^\+|\-|\*|\//,
            },

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

                    var match;
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
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];
        return ctor;
    })();

    NS.Parser = (function() {
        var ctor = function() {
        };

        var methods = {

            _error : function(msg)
            {
                throw msg;
            },

            parseNumber : function(lex)
            {
                var number = lex.consume("@number");
                if (number) 
                    return parseFloat(number[0]);
            },
            parseOperator : function(lex)
            {
                var op = lex.consume("@operator")[0];
                return op;
            },

            parseExpression : function(lex)
            {
                lex.consume("@ws?");
                var first = this.parseNumber(lex) || this._error("Expected number!");
                
                lex.consume("@ws?");
                var operator = this.parseOperator(lex);
                if (operator)
                {
                    lex.consume("@ws?");
                    var second = this.parseNumber(lex);

                    return [first, operator, second];
                }
                else
                    return first;
            },

            parse : function(text)
            {
                var lexer = new NS.Lexer(text);
                var result = this.parseExpression(lexer);
                console.log(result);
            },

        };

        for (var key in methods) {
            ctor.prototype[key] = methods[key];
        }
        return ctor;
    })();

    return NS;
})();



var Context = function(text) 
{
    this.source = text;
    this.symbols = [ {} ];
};
Context.prototype.pushSymbolTable = function()
{
this.symbols.push({});
};
Context.prototype.popSymbolTable = function()
{
this.symbols.pop();
};
Context.prototype.isSymbol = function(a)
{
for (var i = this.symbols.length - 1; i >= 0; --i)
    if (this.symbols[i][a])
    return true;

return false;
};
Context.prototype.empty = function()
{
return this.source.length == 0;
};
Context.prototype.skipWhitespace = function()
{
this.source = this.source.replace(/^\s+/, "");
};
Context.prototype.parseText = function(text, consume)
{
if (this.source.substr(0, text.length) == text) {
    if (consume)
    this.source = this.source.substr(text.length);
    return text;
}
else
    throw "Did not find expected text: '" + text + "' around '" + this.source.substr(0, 80) + "'";
};
Context.prototype.parseRE = function(name, re, consume)
{
var m;
if (m = this.source.match(re)) {
    if (consume)
    this.source = this.source.substr(m[0].length);
    return m[0];
}
else
    throw "Expected "+ name + " at '" + this.source.substr(0, 80) + "'";  
};
Context.prototype._RETable = 
{
    ws : /^\s+/,
    name : /^[_A-Za-z][_A-Za-z0-9]*/,
    complexName : /^[_A-Za-z][_A-Za-z0-9\.]*/,
};
Context.prototype._parseWorker = function(type, consume)
{
var optional = type.match(/\?$/);
if (optional)
    type = type.substr(0, type.length - 1);

try 
{
    if (type.substr(0,1) == "@")
    {
    var re = this._RETable[type.substr(1)];
    return this.parseRE(type, re, consume);
    }
    else {
    return this.parseText(type, consume);
    }
} catch (e)
{
    if (!optional)
    throw e;
}
};
Context.prototype._parseWorker2 = function(types, consume)
{
    var a = [];
    for (var i = 0; i < types.length; ++i)
    a[i] = this._parseWorker(types[i], consume);
    return a;
};
Context.prototype.skip = function(type)
{
// This is not the best method of implementation for 'skip'!
try {
    this._parseWorker(type, true);
} catch (e) 
{
}
};
Context.prototype.consume = function()
{
    return this._parseWorker2(arguments, true);
};
Context.prototype.peek = function()
{
try {
    return this._parseWorker2(arguments, false);
    } catch(e) {}
};

         

var Parser = function () { 
    this._module = null;
    this._functions = [];
};

var methods =
{
    parseModule : function(ctx)
    {
        this._module = ctx.consume("module", "@ws", "@complexName")[2];
        ctx.skip("@ws");
        ctx.consume(";");
    },

    parseArg : function (ctx)
    {
        var arg = {};
        arg.type = ctx.consume("@name", "@ws")[0];
        arg.name = ctx.consume("@name", "@ws?")[0];
        return arg;
    },

    parseExpression : function(ctx)
    {
        var name = ctx.consume("@name")[0];
    },

    parseReturnExpression : function(ctx)
    {
        ctx.consume("return", "@ws");
        this.parseExpression(ctx);
    },

    parseStatementSet : function (ctx)
    {
        ctx.consume("@ws?");
        while (!ctx.peek("}")) {
              
        ctx.consume("@ws?");
                
        var name = ctx.peek("@name")[0];
        if (name == "return")
            console.log(name);

        else 
            throw "Could not parse statement around '" + ctx.source.substr(0, 80) + "'";
        }
    },

    parseFunction : function(ctx)
    {
        var f = {};
        f.args = [];
        f.name = ctx.consume("function", "@ws", "@name", "@ws?", "(")[2];
        ctx.consume("@ws?");
        while (!ctx.peek(")")) {
        f.args.push( this.parseArg(ctx) );
        if (ctx.peek(",")) {
            ctx.consume(",");
            ctx.consume("@ws?");
        }
        };
        ctx.consume(")");

        ctx.consume("@ws?", "->", "@ws?");
        f.returnType = ctx.consume("@name", "@ws?")[0];
        ctx.consume("{", "@ws?");
        this.parseReturnExpression(ctx);
        ctx.consume("}");

        this._functions.push(f);
    },

    parseDefinition : function(ctx)
    {
        while (!ctx.empty())
        {
        var keyword = ctx.peek("@name")[0];
        switch (keyword)
        {
        case "function":
            this.parseFunction(ctx);
            break;
        default: 
            throw "Unrecognized keyword '" + keyword + "' around '" + ctx.source.substr(0, 80)+ "'";
        }
        ctx.consume("@ws?");
        }
    },

    parse : function (text) {
        var ctx = new Context(text);

        ctx.skip("@ws");
        this.parseModule(ctx);
        ctx.skip("@ws");
        this.parseDefinition(ctx);
    },
};

for (var key in methods)
    Parser.prototype[key] = methods[key];
