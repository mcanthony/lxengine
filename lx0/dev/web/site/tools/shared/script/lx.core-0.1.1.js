try { eval("lx"); } catch (e) { lx = {}; }
lx.core = lx.core || {};

(function (NS) {

    //=======================================================================//
    // Math functions
    //=======================================================================//

    NS.random = function (min, max) {
        return Math.random() * (max - min) + min;
    };

    NS.clamp = function (i, n, m) {
        return Math.max(Math.min(i, m), n);
    };

    NS.blend = function (a, b, t) {
        return a * (1 - t) + b * t;
    };

    NS.easeIn = function (x) { return Math.sin(x * Math.PI / 2); };

    NS.easeIn2 = function (x) { NS.easeIn(NS.easeIn(x)); };

    //=======================================================================//
    // String functions
    //=======================================================================//

    NS.trim = function (s) {
        return s.replace(/^\s+|\s+$/g, "")
    };

    //=======================================================================//
    // Array functions
    //=======================================================================//

    NS.each = function (x, f) {
        for (var i in x) {
            f.call(x[i], i, x[i]);
        }
    };

    NS.shuffle = function (a) {        
        
        // Randomly resort the indices
        var b = [];
        for (var i = 0; i < a.length; ++i)
            b.push([ Math.random(), i ]);
        b.sort(function(i, j) { return i[0] - j[0]; });

        // Copy the input into a new re-index array
        var c = [];
        for (var i = 0; i < a.length; ++i)
            c.push(a[b[i][1]]);
        return c;
    };

    //=======================================================================//
    // Code Generation
    //=======================================================================//

    NS.buildClass = function (parent, ctor, members) {

        NS.each(parent.prototype, function (key, value) {
            ctor.prototype[key] = value;
        });
        NS.each(members, function (key, value) {
            ctor.prototype[key] = value;
        });

        return ctor;
    };

    //=======================================================================//
    // Tasks & Timing
    //=======================================================================//

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
            var callback = params.callback || function() {};
            var oncomplete = params.oncomplete || function() {};
            var oncancel = params.oncancel || function() {};

            function worker2(tasks) {

                if (tasks.length > 0) {

                    var group = [];
                    for (var i = 0; i < Math.min(groupSize, tasks.length); ++i)
                        group.push( tasks.shift() );

                    window.setTimeout(function () {
                        
                        for (var i = 0; i < group.length; ++i)
                            group[i]();
                        count += group.length;
                        
                        callback(count);

                        worker2(tasks);
                    }, delay);
                }
                else {
                    if (count == initialLength) 
                        oncomplete();
                    else
                        oncancel(); 
                }
            }
            worker2(tasks);
        }
        return tasks;
    };

})(lx.core);

