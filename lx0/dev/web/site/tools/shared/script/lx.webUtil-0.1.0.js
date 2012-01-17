(function ($) {

    var methods =
    {
        build: function () {
            var anchorNames = [];
            $("h2").each(function (i) {
                var anchor = $("<a/>");
                var name = "toc_" + i;
                anchor.attr("name", name);
                anchorNames.push([name, $(this).text()]);
                $(this).before(anchor);
            });

            var list = $("<ol/>");
            $.each(anchorNames, function (i, value) {
                var item = $("<li/>");
                var link = $("<a/>");
                link.text(value[1]);
                link.attr("href", "#" + value[0]);
                item.html(link);
                list.append(item);
            });
            $(this).html(list);
        },
    };

    $.fn.tableOfContents = function () {

        return this.each(function () {
            methods['build'].call(this);
        });
    };

})(jQuery);

(function ($) {

    function removeLeftIndent(s)
    {
        
        var lines = s.replace(/^\t/g, "    ").split("\n");
        var count = lines[0].length; 
        for (var i = 1; i < lines.length; ++i)
        {
            var m = lines[i].match(/^\s*/);
            if (lines[i].length > m[0].length)
            {
                count = Math.min(count, m[0].length);
            }
        }
        if (count > 0)
        {
            for (var i = 1; i < lines.length; ++i)
            {
                lines[i] = lines[i].substr(count);
            }
        }
        
        return lines.join("\n");
    }

    $.fn.includeSource = function () {
        return this.each(function() {

            var elem = this;
            var url = $(this).attr("data-source");
            var func = $(this).attr("data-function");

            if (url) 
            {
                $.ajax(url + "?rand=" + Math.random(), {
                    dataType : "text",
                    async : true,
                    success : function(text) {
                        $(elem).text(text);
                        $(elem).after("<p><em>Source highlighting provided by <a href='http://softwaremaniacs.org/soft/highlight/en/'>highlight.js</a></em></p>");
                        $(elem).wrap("<code/>").wrap("<pre class='codeview language-javascript'/>").each(function(i, e) { 
                            hljs.highlightBlock(e, '    ', false); 
                        });
                    },
                });
            }
            else if (func) {
                
                var customSource; 
                var javascriptSource;

                eval("customSource = " + func + ".source;");                
                eval("javascriptSource = " + func + ".toString();");

                if (!javascriptSource)
                {
                    console.log("Could not get source for function: ", func);
                    return;
                }

                javascriptSource = removeLeftIndent(javascriptSource);

                var text;
                if (customSource)
                {
                    text = customSource + "\n\n// ==== Javascript translation ====\n\n" + func + " = " + javascriptSource;
                }
                else
                    text = javascriptSource;


                $(elem).text(text);
                $(elem).wrap("<code/>").wrap("<pre class='codeview language-javascript'/>").each(function(i, e) { 
                    hljs.highlightBlock(e, '    ', false); 
                });
            }

        });
    };

})(jQuery);


(function($) {

    $.fn.tableOfContentsMenu = function () {
        return this.each(function() {
            var parent = $("<div/>");
            parent.addClass("layout-row");

            var tocHolder = $("<div style='border: 1px solid #CCC; border-radius: 6px; padding: 8px 2em 8px 8px;  width: 16em; margin-left: 1em;' />");
            tocHolder.append("<strong style='color: #777'>Table of Contents</strong>");
            var tocElem = $("<div/>");
            tocHolder.append(tocElem);
            
            
            $(this).css("max-width", "38em");
            parent.append($(this).clone());
            parent.append(tocHolder);
            $(this).replaceWith(parent);

            tocElem.tableOfContents();
        });
    };
})(jQuery);

function compileWithGoogleClosureCompiler(source, options)
{
    options = $.extend({
        compilation_level : "ADVANCED_OPTIMIZATIONS",
    }, options);

    function submitForm(source)
    {
        var form = $("<form/>");
        form.attr("method", "post");
        form.attr("action", "http://closure-compiler.appspot.com/compile");
        form.attr("target", "_blank");

        var src = $("<input type='hidden' name='js_code' />");
        src.val( source );
        console.log(source);

        form.append( src );
        form.append( $("<input type='hidden' name='output_format' value='text'/>") );
        form.append( $("<input type='hidden' name='output_info' value='compiled_code'/>") );
        form.append( $("<input type='hidden' name='compilation_level' value='" + options.compilation_level + "'/>") );

        $("body").append(form);
        form.submit();
    }

    if (source.match(/\.js$/i))
    {
        
        $.ajax(source + "?rand=" + Math.random(), {
            dataType : "text",
            async : true,
            success : function(text) {
                submitForm(text);
            }
        });
    }
    else
        submitForm(source);
}

function acquireCache (name)
{
    var Cache = (function() {

        var ctor = function(name) {
            this.data = {};
            this._name = name;
        };

        var util = {
            tryEval : function (json, def)
            {
                if (json) {
                    try {
                        var value;
                        eval("value = " + json + ";");
                        if (value !== undefined)
                            return value;
                    }
                    catch (e) {
                        console.log("Exception evaluating '" + json + "'");
                    }
                }
                return def;
            }
        };

        var publicMethods = {
            load : function () 
            {
                this.data = util.tryEval(localStorage[this._name], {});
            },
            save : function () 
            {
                localStorage[this._name] = JSON.stringify(this.data);
            },
            clear : function() 
            {                
                delete localStorage[this._name];
                this.data = {};
            }
        };

        for (var name in publicMethods)
            ctor.prototype[name] = publicMethods[name];
        return ctor;
    })();

    if (localStorage !== undefined)
    {
        var cache = new Cache(name);
        cache.load();
        return cache;
    } 
    else
    {
        var cache = new Cache(name);
        cache.load = function() {};
        cache.save = function() {};
        cache.clear = function() { this.data = {}; }
        return cache;
    }
};

function includeScriptFiles ()
{   
    function evalScript(text)
    {
        try {
            eval(text);
        } catch (e)
        {
            console.log(text);
            console.log("ERROR: Error in evaluating code");
            throw e;
        }
    }
    
    var lxparser;
    var jsgenerator;

    var cache = acquireCache('genericScriptCache');
    for (var i = 0; i < arguments.length; ++i)
    {
        var script = arguments[i];

        if (cache.data[script])
        {
            console.log("Using cached translation for '" + script + "'");
            evalScript(cache.data[script].jscode);
        }
        else if (script.match(/.\lxlang$/i))
        {
            lxparser = lxparser || new lxlang2.Parser();
            jsgenerator = jsgenerator || new lxlang2.GenerateJavascript();

            var url = script + "?rand=" + Math.random();
            $.ajax(url, {
                    dataType : "text",
                    async : false,
                    success : function(text) {
                                                    
                        var ast = lxparser.parse(text);
                        var jscode = jsgenerator.translate(ast);
                        
                        console.log("Compiling script for '" + script + "'");
                        evalScript(jscode);
                        cache.data[script] = { jscode : jscode };
                    }
                });
        }
        else
        {
            var s = "<script type='text/javascript' src='";
            s += script;
            s += "'></script>";
            $("head").append(s);
        }
    }

    cache.save();
}
