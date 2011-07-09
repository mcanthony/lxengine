(function() {
    

    function wind(value)
    {
        value = value.toLowerCase();

        var vel_table = { 
            "none": [ 0.0, 0.0 ],
            "calm": [ 0.0, 0.277 ],
            "light air": [ 0.277, 1.389 ],
            "light breeze": [ 1.389, 3.055 ],
            "gentle breeze": [ 3.055, 5.278 ],
            "moderate breeze": [ 5.278, 8.056 ],
            "fresh breeze": [ 8.056, 10.556 ],
            "strong breeze": [ 10.556, 14.167 ],
            "near gale": [ 14.167, 16.944 ],
            "gale": [ 16.944, 20.556 ],
            "strong gale": [ 20.556, 23.889 ],
            "whole gale": [ 23.889, 28.056 ],
            "storm": [ 28.056, 33.333 ],
            "hurricane": [ 33.333, 50.0 ],
        };

        var dir_table = {
            "north"     : [0, 1, 0],
            "east"      : [1, 0, 0],
            "south"     : [0, -1, 0],
            "west"      : [-1, 0, 0],

            "northeast" : [ 1, 1, 0],
            "northwest" : [-1, 1, 0],
            "southeast" : [ 1,-1, 0],
            "southwest" : [-1,-1, 0],
        };

        var vel;
        var dir;

        var re_str2 = /^(\w+)\s+(\w+)$/;
        var result = re_str2.exec(value);
        if (result) {
            vel = result[1];
            dir = result[2];
        }
        else {
            var re_str3 = /^(\w+) (\w+)\s+(\w+)$/;
            var result = re_str3.exec(value);
            if (result) {
                vel = result[1] + " " + result[2];
                dir = result[3];
            }
        }

        vel = vel_table[vel][1];
        dir = dir_table[dir];

        // TODO: Need to define a mechanism for redirecting the pseudo-attribute
        // to multiple actual attribute calls 
        
        //elem.setAttribute("wind_velocity", vel);
        //elem.setAttribute("wind_direction", dir);

        return false;
    }
    
    //engine.addPsuedoAttribute("wind", wind);

})();
