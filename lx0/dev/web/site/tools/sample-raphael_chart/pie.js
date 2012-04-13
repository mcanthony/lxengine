Raphael.fn.pieChart = function (cx, cy, r, values, labels, descriptions, stroke, animate) {
    
    var paper = this;
    var blocks = paper.set();
    
    var total = 0;
    var start = 50;
    
    var overlay = paper.image("foot.png", 245, 0, 210, 716).attr({ stroke : "none" });
    
    var overFuncs = [];
    var outFuncs = [];
    var starts = [];

    var process = function (j) {
        var value = values[j];
        var description = descriptions[j];
        var percent = 100 * value / total;
        var size = percent * 6;
        var hue = .18 + j * .1 + (j % 5) * 1/5;
        
        var centerX = 350;
        var centerY = start + size/2;
        
        
        starts.push(start);
        
        var color = Raphael.hsb(hue, .85, .9);
        var color2 = Raphael.hsb(hue, 1, 1);
        var bcolor = Raphael.hsb(hue, .5, 1);
        var ms = 250;
        var delta = 30;
        var p = paper.rect(250, start, 200, size).attr({fill: "" + color, stroke: stroke, "stroke-width": 2});            
        
        var txt = paper.text(centerX - 250, centerY, labels[j]);
        txt.attr({fill: bcolor, stroke: "none", opacity: 0, "font-size": 20});
        
        var txt2 = paper.text(200 + centerX, centerY, Math.floor(percent * 10) / 10 + "%");
        txt2.attr({fill: Raphael.hsb(hue, .2, .8), stroke: "none", opacity: 0, "font-size": Math.max(size / 5, 10) });
  
        var txt3 = $("<div/>").text(description);
        txt3.css("position", "absolute");
        txt3.css("left", 0);
        txt3.css("top", 0);
        txt3.css("text-align", "center");
        txt3.css("width", "700px");
        txt3.hide();
        $("#desc").append(txt3);
        
        overFuncs.push(function () {
            blocks.attr({opacity: 1});
            p.stop().animate({transform: "s1.0 1.1 " + cx + " " + centerY, fill: color2 }, ms, "easeIn");
            txt.stop().animate({opacity: 1}, ms, "easeIn");
            txt2.stop().animate({opacity: 1}, ms, "easeIn");
            txt3.children().stop().hide();
            txt3.fadeIn(500);
            overlay.toFront();
        });
        outFuncs.push(function () {
            p.stop().animate({transform: "", fill : color }, ms / 2, "linear");
            txt.stop().animate({opacity: 0}, ms * 4);
            txt2.stop().animate({opacity: 0}, ms * 4);
            txt3.fadeOut(200);
        });           
        
        start += size;
        blocks.push(p);
    };

    
    for (var i = 0; i < values.length; i++)
        total += values[i];
    
    for (i = 0; i < values.length; i++)
        process(i);
    
    overlay.toFront();
    
    function compute_index(e)
    {
        var posy = e.pageY - $(document).scrollTop() - $('#holder').offset().top;
                
        var i;
        for (i = 0; i < starts.length; ++i)
        {
            if (starts[i] >= posy)
                return i - 1;
        }
        return i - 1;
    }
    
    overlay.mousemove(function (e) {

        var i = compute_index(e);
        console.log(i, this._gindex);
        
        if (this._gindex !== i)
        {
            console.log("Show " + i, "Hide " + this._gindex);
            if (this._gindex !== undefined)
                outFuncs[this._gindex]();
            overFuncs[i]();
            this._gindex = i;
        }
    });
    overlay.mouseout(function () {
        if (this._gindex !== undefined)
            outFuncs[this._gindex]();
        delete this._gindex;
    });  
     
    if (animate)
    {
        blocks.attr({ opacity : 0.25 });
        var anim = Raphael.animation({opacity : 1}, 1000, "easeIn");
        blocks.animate(anim.delay(1500));
    }
    return undefined;
};

$(function () {
    function setup(animate) 
    {
        $("#holder").html("");
        
        var values = [];
        var labels = [];
        var descs = [];
        
        $("tr").each(function () {
            var value = parseInt( $(this).children().eq(1).text(), 10);
            var desc = $(this).children().eq(2).text();
        
            values.push(value);
            labels.push($("th", this).text());
            descs.push(desc);
        });
        $("table").hide();
        Raphael("holder", 700, 700).pieChart(350, 350, 200, values, labels, descs, "#333", animate);
    }
    
    setup(true);
    
    function create_handler(name, factor) {
        return function() {
            var value = parseInt($(name).text(), 10);
            value *= factor;
            $(name).text(value);
            setup(false);
        } 
    }
    
    
    $("#add_cpp").click( create_handler("#cpp_data", 1.2) );
    $("#add_js").click( create_handler("#javascript_data", 1.2) );
    $("#remove_cpp").click( create_handler("#cpp_data", .8) );
    $("#remove_js").click( create_handler("#javascript_data", .8) );
    

});
