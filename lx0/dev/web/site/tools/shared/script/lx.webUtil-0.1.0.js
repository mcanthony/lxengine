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

function includeScriptFiles ()
{   
    var lxparser;
    var jsgenerator;


    for (var i = 0; i < arguments.length; ++i)
    {
        var url = arguments[i];
        var script = url + "?rand=" + Math.random();

        if (url.match(/.\lxlang$/i))
        {
            lxparser = lxparser || new lxlang2.Parser();
            jsgenerator = jsgenerator || new lxlang2.GenerateJavascript();

            $.ajax(script, {
                    dataType : "text",
                    async : false,
                    success : function(text) {
                                                    
                        var ast = lxparser.parse(text);
                        var jscode = jsgenerator.translate(ast);
                        
                        try {
                            eval(jscode);
                        } catch (e)
                        {
                            console.log(jscode);
                            console.log("ERROR: Error in translated LxLang code");
                            throw e;
                        }
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

}
