Raphael.fn.pieChart = function (cx, cy, r, values, labels, descriptions, stroke, animate) {
    
    var paper = this;
    var rad = Math.PI / 180;
    var chart = paper.set();
    var blocks = paper.set();
    
    var total = 0;
    var start = 50;
    
    var icon = "M20.755,1H10.62C9.484,1,8.562,1.921,8.562,3.058v24.385c0,1.136,0.921,2.058,2.058,2.058h10.135c1.136,0,2.058-0.922,2.058-2.058V3.058C22.812,1.921,21.891,1,20.755,1zM14.659,3.264h2.057c0.101,0,0.183,0.081,0.183,0.18c0,0.1-0.082,0.18-0.183,0.18h-2.057c-0.1,0-0.181-0.081-0.181-0.18C14.478,3.344,14.559,3.264,14.659,3.264zM13.225,3.058c0.199,0,0.359,0.162,0.359,0.36c0,0.198-0.161,0.36-0.359,0.36c-0.2,0-0.36-0.161-0.36-0.36S13.025,3.058,13.225,3.058zM15.688,28.473c-0.796,0-1.44-0.646-1.44-1.438c0-0.799,0.645-1.439,1.44-1.439s1.44,0.646,1.44,1.439S16.483,28.473,15.688,28.473zM22.041,24.355c0,0.17-0.139,0.309-0.309,0.309H9.642c-0.17,0-0.308-0.139-0.308-0.309V6.042c0-0.17,0.138-0.309,0.308-0.309h12.09c0.17,0,0.309,0.138,0.309,0.309V24.355z";
    icon = "M14.296,27.885v-2.013c0,0-0.402-1.408-1.073-2.013c-0.671-0.604-1.274-1.274-1.409-1.61c0,0-0.268,0.135-0.737-0.335s-1.812-2.616-1.812-2.616l-0.671-0.872c0,0-0.47-0.671-1.275-1.342c-0.805-0.672-0.938-0.067-1.476-0.738s0.604-1.275,1.006-1.409c0.403-0.134,1.946,0.134,2.684,0.872c0.738,0.738,0.738,0.738,0.738,0.738l1.073,1.141l0.537,0.201l0.671-1.073l-0.269-2.281c0,0-0.604-2.55-0.737-4.764c-0.135-2.214-0.47-5.703,1.006-5.837s1.007,2.55,1.073,3.489c0.067,0.938,0.806,5.232,1.208,5.568c0.402,0.335,0.671,0.066,0.671,0.066l0.402-7.514c0,0-0.479-2.438,1.073-2.549c0.939-0.067,0.872,1.543,0.872,2.147c0,0.604,0.269,7.514,0.269,7.514l0.537,0.135c0,0,0.402-2.214,0.604-3.153s0.604-2.416,0.537-3.087c-0.067-0.671-0.135-2.348,1.006-2.348s0.872,1.812,0.939,2.415s-0.134,3.153-0.134,3.757c0,0.604-0.738,3.623-0.537,3.824s2.08-2.817,2.349-3.958c0.268-1.141,0.201-3.02,1.408-2.885c1.208,0.134,0.47,2.817,0.402,3.086c-0.066,0.269-0.671,2.349-0.872,2.952s-0.805,1.476-1.006,2.013s0.402,2.349,0,4.629c-0.402,2.281-1.61,5.166-1.61,5.166l0.604,2.08c0,0-1.744,0.671-3.824,0.805C16.443,28.221,14.296,27.885,14.296,27.885z";
    //var overlay = paper.path(icon).attr({fill: "#000", stroke: "none", transform: "t 300 330 s16 26"});
    var overlay = paper.image("foot.png", 245, 0, 210, 716).attr({ stroke : "none" });
    chart.push(overlay); 
    
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
            p.stop().toFront().animate({transform: "s1.0 1.1 " + cx + " " + centerY, fill: color2 }, ms, "easeIn");
            txt.stop().toFront().animate({opacity: 1}, ms, "easeIn");
            txt2.stop().toFront().animate({opacity: 1}, ms, "easeIn");
            txt3.children().stop().hide();
            txt3.fadeIn(500);
            overlay.toFront();
        });
        outFuncs.push(function () {
            p.stop().animate({transform: "", fill : color }, ms / 2, "linear");
            txt.stop().animate({opacity: 0}, ms * 4);
            txt2.stop().animate({opacity: 0}, ms * 4);
            txt3.fadeOut(200);
            overlay.toFront();
        });           
        
        start += size;
        chart.push(p);
        chart.push(txt);
        chart.push(txt2);
        blocks.push(p);

    };
   

 

    
    for (var i = 0; i < values.length; i++) {
        total += values[i];
    }
    for (i = 0; i < values.length; i++) {
        process(i);
    }
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
       overlay.mouseover(function (e) {           
        }).mouseout(function () {
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
    return chart;
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
