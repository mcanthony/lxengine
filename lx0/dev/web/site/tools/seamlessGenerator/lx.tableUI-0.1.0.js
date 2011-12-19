//
// Table-UI: a lightweight, functional automatic UI generator for rapid prototyping
//
var tableUI =
{
    _generators : 
    {
        _label : function(data)
        {
            var td = $("<td/>");
            var span = $("<span/>");
            span.css("padding-right", "1em");
            span.text(data.label);
            td.append(span);
            return td;
        },

        seed : function (ui, options, data)
        {
            var value;

            var tr = $("<tr/>");
            tr.append( this._label(data) );
                        
            {
                var td = $("<td/>");
                value = $("<input/>");
                value.attr("type", "input");
                value.val(options[data.label]);

                var button = $("<input/>");
                button.attr("type", "button");
                button.val("random");
                button.click(function() {
                    value.val( Math.floor(Math.random() * 10000) ).trigger('change');
                });

                td.append(value);
                td.append(button);
                tr.append(td);

                value.change(function () {                    var value = $(this).val();                    options[data.label] = value;                    ui.onchange(value);                });            }
            return tr;
        },

        "int" : function (ui, options, data)
        {
            var tr = $("<tr/>");
            tr.append( this._label(data) );

            {
                var td = $("<td/>");
                value = $("<input/>");
                value.attr("type", "input");
                value.val(options[data.label]);
                            
                td.append(value);
                tr.append(td);

                value.change(function () {                    var value = parseInt($(this).val());                    options[data.label] = value;                    ui.onchange(value);                });            }
            return tr;
        },

        "string" : function (ui, options, data) 
        {
            var tr = $("<tr/>");
            tr.append( this._label(data) );
            {
                var td = $("<td/>");
                value = $("<input/>");
                value.attr("type", "input");
                value.val(options[data.label]);
                            
                td.append(value);
                tr.append(td);

                value.change(function () {                    var value = $(this).val();                    options[data.label] = value;                    ui.onchange(value);                });            }
            return tr;
        },

        "float" : function (ui, options, data)
        {
            var tr = $("<tr/>");
            tr.append( this._label(data) );

            {
                var td = $("<td/>");
                value = $("<input/>");
                value.attr("type", "input");
                value.val(options[data.label]);
                            
                td.append(value);
                tr.append(td);

                value.change(function () {                    var value = parseFloat($(this).val());                    options[data.label] = value;                    ui.onchange(value);                });            }
            return tr;
        }
    },

    generate : function(anchor, options, elements, params)
    {
        var ui = 
        {
            onchange : params.onchange,
        };

        var _this = this;
        $.each(elements, function(i,data) {
            var tr = _this._generators[data.type](ui, options, data);
            $(anchor).append(tr);
        });
    },
};