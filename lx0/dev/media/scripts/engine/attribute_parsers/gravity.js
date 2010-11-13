(function () {
    function gravity(s) {
        if (typeof s == "string") {
            switch (s.toLowerCase()) {
                // Source : http://en.wikipedia.org/wiki/Gravity_of_Earth       
                case "on": return [0, 0, -9.82];
                case "off": return [0, 0, 0];
                case "earth": return [0, 0, -9.82];
                case "moon": return [0, 0, -1.625];
                case "mars": return [0, 0, -3.728];
                case "jupiter": return [0, 0, -25.93];
                case "sun": return [0, 0, -274.1];
                case "mercury": return [0, 0, -3.703];
                case "venus": return [0, 0, -8.872];
                case "saturn": return [0, 0, -11.19];
                case "neptune": return [0, 0, -11.28];
                case "pluto": return [0, 0, -0.610];
                case "uranus": return [0, 0, -9.01];
                default: return undefined;
            }
        }
        else if (typeof s == "number") {
            return [0, 0, s];
        }
        else
            return undefined;
    }
    engine.addAttributeParser("gravity", gravity);
})();
