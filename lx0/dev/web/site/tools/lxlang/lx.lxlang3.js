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
                    else if (terminal.reArr)
                    {
                        for (var i = 0; i < terminal.reArr.length; ++i)
                        {
                            if (m = terminal.reArr[i].exec(text)) {
                                setMatch(key, terminal, m[0]);
                            }
                        }
                    }
                    else if (terminal.arr)
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

        var ctor = function(text)
        {
            this._source = text;
            this._index = 0;
            this._tokens = _parseTokens(text);
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

            consume : function() {},
            peek : function() {},
        };

        for (var key in methods)
            ctor.prototype[key] = methods[key];

        return ctor;
    })();

})(lx.lxlang3);
