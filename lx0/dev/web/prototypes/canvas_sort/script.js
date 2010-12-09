
var unsortedData = [];
var dataSet = [];

for (var i = 0; i < 512; ++i) {
    unsortedData.push(Math.floor(Math.random() * 255));
}

function reset() {
    dataSet = [];
    for (var i = 0; i < 512; ++i)
        dataSet[i] = unsortedData[i];
}

function draw() {
    for (var x = 0; x < 512; ++x) {
        var c = dataSet[x];
        ctx.fillStyle = "rgb(" + c + "," + c + "," + 230 + ")";
        ctx.fillRect(x, 0, 1, 32);
    }
}

function bubbleSort() {
    function inner(i) {
        for (var j = 0; j < (dataSet.length - 1); j++) {
            if (dataSet[j] > dataSet[j + 1]) {
                var t = dataSet[j + 1];
                dataSet[j + 1] = dataSet[j];
                dataSet[j] = t;
            }
        }

        // Unfortunately, the nested loop implementation of the bubble sort
        // needs to be broken into a tail recursion implementation via
        // setTimeout.  Otherwise, the canvas won't update between each iteration.
        if (i < dataSet.length) {
            draw();
            setTimeout(function () { inner(++i); }, 1);
        }
    }

    inner(0);
}
