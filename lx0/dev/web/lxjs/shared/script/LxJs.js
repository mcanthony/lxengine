var LxJs = {};
LxJs.Media = {};

LxJs.Media.GradientSet =
{
    "softreds" : [
        [ 9, ["#931b03", "#efcbc4"]],
        [.4, ["#6b695a", "#efeac4"]],
        [.5, ["#820e1e", "#6f343c"]],
        [.1, ["#b52128", "#e34025"]],
    ],

    "bluebird" : [
        [50, ["#767b87", "#254de3"]],
        [50, ["#5088ac", "#0428ab"]],
        [5, ["#bfd05a", "#bfd0db"]],
        [1, ["#efe1dd", "#b82d07"]],
    ],

    "coppertone" : [
        [1, ["#825c52", "#47332d"]],
        [1, ["#572112", "#77685a"]],
        [1, ["#c87e35", "#47210b"]],
        [1, ["#35242c", "#958364"]],
    ],

    "forest" : [
        [40, ["#254d0f", "#739362"]],
        [40, ["#363825", "#366925"]],
        [3, ["#8f6925", "#dcceb4"]],
        [4, ["#47433b", "#52643b"]],
        [8, ["#103b2e", "#153b24"]],
    ],
};

LxJs._url_param_table = null;
LxJs.url_param =  function (name, default_value) {
        if (this._url_param_table == null) {
            this._url_param_table = {};
            var url = window.location.href;
            var params = url.split("\?", 2)[1];
            if (typeof params != "undefined") {
                var pairs = params.split("\&");
                for (var i in pairs) {
                    var p = pairs[i].split("=", 2);
                    this._url_param_table[p[0]] = unescape(p[1]);
                }
            }
        }

        if (typeof this._url_param_table[name] != "undefined")
            return this._url_param_table[name];
        else
            return default_value;
    };

LxJs.run_tasks = function(tasks, delay) {
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
      };

LxJs.rand = function (min, max) {
        return Math.random() * (max - min) + min;
      };
LxJs.mix = function (a, b, t) {
        return a * (1 - t) + b * t;
      };
LxJs.color_hex_to_array = function (s) {
        function R(s) { return parseInt(s.substring(1, 3), 16); };
        function G(s) { return parseInt(s.substring(3, 5), 16); };
        function B(s) { return parseInt(s.substring(5, 7), 16); };
        return [R(s), G(s), B(s)];
      };

LxJs.rgb_from_gradient_set = function (gradient_set) {

        function choose_gradient() {
          //
          // Presume we're dealing with small lists, so just do two linear searches for
          // simplicity's sake.
          //
          var sum = 0;
          for (var i = 0; i < gradient_set.length; ++i) {
            sum += gradient_set[i][0];
          }

          var r = LxJs.rand(0, sum);
          sum = 0;
          for (var i = 0; i < gradient_set.length; ++i) {
            sum += gradient_set[i][0];
            if (r <= sum)
              return gradient_set[i][1];
          }
          return gradient_set[gradient_set.length - 1][1];
        }


        with (LxJs) {
          var gradient = choose_gradient();
          var c0 = color_hex_to_array(gradient[0]);
          var c1 = color_hex_to_array(gradient[1]);
          var t = rand(0, 1);
          var r = Math.floor(mix(c0[0], c1[0], t));
          var g = Math.floor(mix(c0[1], c1[1], t));
          var b = Math.floor(mix(c0[2], c1[2], t));

          return "rgb(" + r + ", " + g + ", " + b + ")";
        }
      };

