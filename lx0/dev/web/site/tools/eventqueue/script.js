//
// Custom events
//
var count = 0;
var test = {

    count_odds: function () {
        var print = co.func(function (time) {
            $("#output").append("Odd  = " + count + "  (" + time + ")\n");
            count++;
        });
        return co.sequence(
            co.wait(1000),
            co.repeat(10,
                co.sequence(print, co.wait(2000))
            )
        );
    },

    count_evens: function () {
        var print = co.func(function (time) {
            $("#output").append("Even = " + count + "  (" + time + ")\n");
            count++;
        });
        return co.repeat(10,
            co.sequence(print, co.wait(2000))
        );
    }
};


//
// "Entry-point"
//
function main() {
    var engine = new Engine();
    engine.addEvent(0, test.count_odds());
    engine.addEvent(0, test.count_evens());
    engine.run();
}