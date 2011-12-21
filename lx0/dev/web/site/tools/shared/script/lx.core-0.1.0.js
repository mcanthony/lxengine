
try { eval("lx"); } catch (e) { lx = {}; }

lx.core = (function () {
   
    var NS = {};

    NS.clamp = function(i, n, m) {
        return Math.max(Math.min(i, m), n);
    };
    NS.blend = function (a, b, t) {
        return a * (1 - t) + b * t;
    };
    NS.random = function (min, max) {
        return Math.random() * (max - min) + min;
    };

    NS.trim = function(s) {
        return s.replace(/^\s+|\s+$/g,"")    
    };


    /*
        Takes an array of function objects and runs them with the
        specified delay between each call. 
     */
    NS.run_tasks = function(tasks, delay, params) {
        delay = delay || 0;
        if (!params)
        {
            function worker(tasks) {
                if (tasks.length > 0) {
                    var task = tasks.shift();
                    window.setTimeout(function () {
                        task();
                        worker(tasks);
                    }, delay);
                }
            }
            worker(tasks);
        }
        else
        {
            var count = 0;
            var initialLength = tasks.length;
            var groupSize = params.groupSize || 5;

            function worker(tasks) {
                if (tasks.length > 0) {

                    var group = [];
                    for (var i = 0; i < Math.min(groupSize, tasks.length); ++i)
                        group.push( tasks.shift() );

                    window.setTimeout(function () {
                        
                        for (var i = 0; i < group.length; ++i)
                            group[i]();
                        count += group.length;
                        
                        if (params.callback)
                            params.callback(count);

                        worker(tasks);
                    }, delay);
                }
                else {
                    if (count == initialLength) 
                        if (params.oncomplete)
                            params.oncomplete();
                    else
                        if (params.oncancel)
                            params.oncancel(); 
                }
            }
            worker(tasks);
        }
        return tasks;
    };

    return NS;

})();

