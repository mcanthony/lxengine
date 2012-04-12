Raphael.fn.pieChart = function (cx, cy, r, values, labels, descriptions, stroke) {
    
    var paper = this;
    var rad = Math.PI / 180;
    var chart = paper.set();
    
    var total = 0;
    var start = 50;

    var process = function (j) {
            var value = values[j];
            var description = descriptions[j];
            var percent = 100 * value / total;
            var size = percent * 6;
            var hue = .18 + j * .1 + (j % 5) * 1/5;
            
            var centerX = 350;
            var centerY = start + size/2;
            
            
            var color = Raphael.hsb(hue, .75, 1);
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
            
            p.mouseover(function () {
                p.stop().toFront().animate({transform: "s1.3 1.1 " + cx + " " + centerY }, ms, "easeIn");
                txt.stop().toFront().animate({opacity: 1}, ms, "easeIn");
                txt2.stop().toFront().animate({opacity: 1}, ms, "easeIn");
                txt3.children().stop().hide();
                txt3.fadeIn(500);
            }).mouseout(function () {
                p.stop().animate({transform: "" }, ms / 2, "linear");
                txt.stop().animate({opacity: 0}, ms * 4);
                txt2.stop().animate({opacity: 0}, ms * 4);
                txt3.fadeOut(200);
            });           
            
            start += size;
            chart.push(p);
            chart.push(txt);
            chart.push(txt2);
            start += .1;
        };
    for (var i = 0; i < values.length; i++) {
        total += values[i];
    }
    for (i = 0; i < values.length; i++) {
        process(i);
    }
    return chart;
};

$(function () {
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
    Raphael("holder", 700, 700).pieChart(350, 350, 200, values, labels, descs, "#333");
});
