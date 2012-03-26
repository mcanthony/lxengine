function pluginDirt() {
    var plugin =
    {
        options : 
        {
            seed : Math.floor(Math.random() * 10000),
            background : "#5d381f",

            clusters : 5000,
            bladesMin : 2,
            bladesMax : 8,
            bladeWidth: 0.01,
            dirtColor0 : "#512f0b",
            dirtColor1: "#804e1a",
        },

        ui :
        [
            { 
                type : "seed",
                label: "seed", 
            },
            {
                type : "string",
                label : "dirtColor0",
            },
            {
                type : "string",
                label : "dirtColor1",
            },
        ],

        drawTasks : function (ctx, drawTiled)
        {
            var opt = this.options;
            var tasks = [];

            //
            // Set up the initial state
            //
            tasks.push(function() {
                ctx.lineCap = "round";
                ctx.lineWidth = 0.07;
            });



            //
            // Draw a dirt background
            //
            for (var i = 0; i < 100; ++i) {
                tasks.push(function() {                            
                    var x = -.5 + Math.random();
                    var y = -.5 + Math.random();
                    var dx = -.05 + Math.random() * .1;
                    var dy = -.05 + Math.random() * .1;
                    var m = Math.random();    
                
                    ctx.strokeStyle = lx.color.mix("#504030", "#5d381f", m).html();
               
                    drawTiled(x, y, function (x, y) {
                        ctx.beginPath();
                        ctx.moveTo(x, y);
                        ctx.lineTo(x + dx, y + dy);
                        ctx.stroke();
                    });
                });
            }

            tasks.push(function() {
                ctx.lineCap = "square";
                ctx.lineWidth = 0.01;
            });


            console.log(opt);
            //
            // Draw a dirt background
            //
            for (var i = 0; i < 10000; ++i) {
                tasks.push(function() {                            
                    var x = -.5 + Math.random();
                    var y = -.5 + Math.random();
                    var dx = -.015 + Math.random() * .03;
                    var dy = -.015 + Math.random() * .03;
                    var m = Math.random();    
                
                    ctx.strokeStyle = lx.color.mix(opt.dirtColor0, opt.dirtColor1, m).html();
               
                    drawTiled(x, y, function (x, y) {
                        ctx.beginPath();
                        ctx.moveTo(x, y);
                        ctx.lineTo(x + dx, y + dy);
                        ctx.stroke();
                    });
                });
            }

            return tasks;
        },
    };  
    return plugin;
}