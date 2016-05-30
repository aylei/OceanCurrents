/**
 * Created by Danko on 2016/5/21.
 */
var _mine = function() {
    "use strict";

    var τ = 2 * Math.PI;
    var H = 0.0000360;
    var DEFAULT_CONFIG = "current/ocean/surface/currents/orthographic";
    var TOPOLOGY = "/data/ne_110m_land.json";

    function isTruthy(x) {
        return !!x;
    }

    function isValue(x) {
        return x !== null && x !== undefined;
    }

    function coalesce(a, b) {
        return isValue(a) ? a : b;
    }

    function floorMod(a, n) {
        var f = a - n * Math.floor(a / n);
        return f === n ? 0 : f;
    }

    function distance(a, b) {
        var Δx = b[0] - a[0];
        var Δy = b[1] - a[1];
        return Math.sqrt(Δx * Δx + Δy * Δy);
    }

    function clamp(x, low, high) {
        return Math.max(low, Math.min(x, high));
    }

    function proportion(x, low, high) {
        return (_mine.clamp(x, low, high) - low) / (high - low);
    }

    function spread(p, low, high) {
        return p * (high - low) + low;
    }

    function zeroPad(n, width) {
        var s = n.toString();
        var i = Math.max(width - s.length, 0);
        return new Array(i + 1).join("0") + s;
    }

    function capitalize(s) {
        return s.length === 0 ? s : s.charAt(0).toUpperCase() + s.substr(1);
    }

    function isEmbeddedInIFrame() {
        return window != window.top;
    }

    function ymdRedelimit(ymd, fromDelimiter, toDelimiter) {
        if (!fromDelimiter) {
            return ymd.substr(0, 4) + toDelimiter + ymd.substr(4, 2) + toDelimiter + ymd.substr(6, 2);
        }
        var parts = ymd.substr(0, 10).split(fromDelimiter);
        return [parts[0], parts[1], parts[2]].join(toDelimiter);
    }

    function dateToUTCymd(date, delimiter) {
        return ymdRedelimit(date.toISOString(), "-", delimiter || "");
    }

    function dateToConfig(date) {
        return {date: _mine.dateToUTCymd(date, "/"), hour: _mine.zeroPad(date.getUTCHours(), 2) + "00"};
    }

    function log() {
        function format(o) { return o && o.stack ? o + "\n" + o.stack : o; }
        return {
            debug:   function(s) { if (console && console.log) console.log(format(s)); },
            info:    function(s) { if (console && console.info) console.info(format(s)); },
            error:   function(e) { if (console && console.error) console.error(format(e)); },
            time:    function(s) { if (console && console.time) console.time(format(s)); },
            timeEnd: function(s) { if (console && console.timeEnd) console.timeEnd(format(s)); }
        };
    }

    function view() {
        var w = window;
        var d = document && document.documentElement;
        var b = document && document.getElementsByTagName("body")[0];
        var x = w.innerWidth || d.clientWidth || b.clientWidth;
        var y = w.innerHeight || d.clientHeight || b.clientHeight;
        return {width: x, height: y};
    }

    function removeChildren(element) {
        while (element.firstChild) {
            element.removeChild(element.firstChild);
        }
    }

    function clearCanvas(canvas) {
        canvas.getContext("2d").clearRect(0, 0, canvas.width, canvas.height);
        return canvas;
    }

    function colorInterpolator(start, end) {
        var r = start[0], g = start[1], b = start[2];
        var Δr = end[0] - r, Δg = end[1] - g, Δb = end[2] - b;
        return function(i, a) {
            return [Math.floor(r + i * Δr), Math.floor(g + i * Δg), Math.floor(b + i * Δb), a];
        };
    }


    function asColorStyle(r, g, b, a) {
        return "rgba(" + r + ", " + g + ", " + b + ", " + a + ")";
    }

    function windIntensityColorScale(step, maxWind) {
        var result = [];
        for (var j = 85; j <= 255; j += step) {
            result.push(asColorStyle(j, j, j, 1.0));
        }
        result.indexFor = function(m) {  // map wind speed to a style
            return Math.floor(Math.min(m, maxWind) / maxWind * (result.length - 1));
        };
        return result;
    }

    function segmentedColorScale(segments) {
        var points = [], interpolators = [], ranges = [];
        for (var i = 0; i < segments.length - 1; i++) {
            points.push(segments[i+1][0]);
            interpolators.push(colorInterpolator(segments[i][1], segments[i+1][1]));
            ranges.push([segments[i][0], segments[i+1][0]]);
        }

        return function(point, alpha) {
            var i;
            for (i = 0; i < points.length - 1; i++) {
                if (point <= points[i]) {
                    break;
                }
            }
            var range = ranges[i];
            return interpolators[i](_mine.proportion(point, range[0], range[1]), alpha);
        };
    }

    function loadJson(resource) {
        var d = when.defer();
        d3.json(resource, function(error, result) {
            return error ?
                !error.status ?
                    d.reject({status: -1, message: "Cannot load resource: " + resource, resource: resource}) :
                    d.reject({status: error.status, message: error.statusText, resource: resource}) :
                d.resolve(result);
        });
        return d.promise;
    }

    function distortion(projection, λ, φ, x, y) {
        var hλ = λ < 0 ? H : -H;
        var hφ = φ < 0 ? H : -H;
        var pλ = projection([λ + hλ, φ]);
        var pφ = projection([λ, φ + hφ]);

        var k = Math.cos(φ / 360 * τ);

        return [
            (pλ[0] - x) / hλ / k,
            (pλ[1] - y) / hλ / k,
            (pφ[0] - x) / hφ,
            (pφ[1] - y) / hφ
        ];
    }

    function newAgent(initial) {

        function cancelFactory() {
            return function cancel() {
                cancel.requested = true;
                return agent;
            };
        }

        function runTask(cancel, taskAndArguments) {

            function run(args) {
                return cancel.requested ? null : _.isFunction(task) ? task.apply(agent, args) : task;
            }

            function accept(result) {
                if (!cancel.requested) {
                    value = result;
                    agent.trigger("update", result, agent);
                }
            }

            function reject(err) {
                if (!cancel.requested) {
                    agent.trigger("reject", err, agent);
                }
            }

            function fail(err) {
                agent.trigger("fail", err, agent);
            }

            try {
                var task = taskAndArguments[0];
                when.all(_.rest(taskAndArguments)).then(run).then(accept, reject).done(undefined, fail);
                agent.trigger("submit", agent);
            } catch (err) {
                fail(err);
            }
        }

        var value = initial;
        var runTask_debounced = _.debounce(runTask, 0);
        var agent = {

            value: function() {
                return value;
            },

            cancel: cancelFactory(),

            submit: function(task, arg0, arg1, and_so_on) {
                this.cancel();
                runTask_debounced(this.cancel = cancelFactory(), arguments);
                return this;
            }
        };

        return _.extend(agent, Backbone.Events);
    }

    function parse(hash, projectionNames, overlayTypes) {
        var option, result = {};
        var tokens = /^(current|(\d{4})\/(\d{1,2})\/(\d{1,2})\/(\d{3,4})Z)\/(\w+)\/(\w+)\/(\w+)([\/].+)?/.exec(hash);
        if (tokens) {
            var date = tokens[1] === "current" ?
                "current" :
                tokens[2] + "/" + zeroPad(tokens[3], 2) + "/" + zeroPad(tokens[4], 2);
            var hour = isValue(tokens[5]) ? zeroPad(tokens[5], 4) : "";
            result = {
                date: date,
                hour: hour,
                param: tokens[6],
                surface: tokens[7],
                level: tokens[8],
                projection: "orthographic",
                orientation: "",
                topology: TOPOLOGY,
                overlayType: "default",
                showGridPoints: false
            };
            coalesce(tokens[9], "").split("/").forEach(function(segment) {
                if ((option = /^(\w+)(=([\d\-.,]*))?$/.exec(segment))) {
                    if (projectionNames.has(option[1])) {
                        result.projection = option[1];
                        result.orientation = coalesce(option[3], "");
                    }
                }
                else if ((option = /^overlay=(\w+)$/.exec(segment))) {
                    if (overlayTypes.has(option[1]) || option[1] === "default") {
                        result.overlayType = option[1];
                    }
                }
                else if ((option = /^grid=(\w+)$/.exec(segment))) {
                    if (option[1] === "on") {
                        result.showGridPoints = true;
                    }
                }
            });
        }
        return result;
    }

    var Configuration = Backbone.Model.extend({
        id: 0,
        _ignoreNextHashChangeEvent: false,
        _projectionNames: null,
        _overlayTypes: null,

        toHash: function() {
            var attr = this.attributes;
            var dir = attr.date === "current" ? "current" : attr.date + "/" + attr.hour + "Z";
            var proj = [attr.projection, attr.orientation].filter(isTruthy).join("=");
            var ol = !isValue(attr.overlayType) || attr.overlayType === "default" ? "" : "overlay=" + attr.overlayType;
            var grid = attr.showGridPoints ? "grid=on" : "";
            return [dir, attr.param, attr.surface, attr.level, ol, proj, grid].filter(isTruthy).join("/");
        },

        sync: function(method, model, options) {
            switch (method) {
                case "read":
                    if (options.trigger === "hashchange" && model._ignoreNextHashChangeEvent) {
                        model._ignoreNextHashChangeEvent = false;
                        return;
                    }
                    model.set(parse(
                        window.location.hash.substr(1) || DEFAULT_CONFIG,
                        model._projectionNames,
                        model._overlayTypes));
                    break;
                case "update":
                    model._ignoreNextHashChangeEvent = true;
                    window.location.hash = model.toHash();
                    break;
            }
        }
    });

    function buildConfiguration(projectionNames, overlayTypes) {
        var result = new Configuration();
        result._projectionNames = projectionNames;
        result._overlayTypes = overlayTypes;
        return result;
    }

    return {
        isTruthy: isTruthy,
        isValue: isValue,
        coalesce: coalesce,
        floorMod: floorMod,
        distance: distance,
        clamp: clamp,
        proportion: proportion,
        spread: spread,
        zeroPad: zeroPad,
        capitalize: capitalize,
        isEmbeddedInIFrame: isEmbeddedInIFrame,
        ymdRedelimit: ymdRedelimit,
        dateToUTCymd: dateToUTCymd,
        dateToConfig: dateToConfig,
        log: log,
        view: view,
        removeChildren: removeChildren,
        clearCanvas: clearCanvas,
        windIntensityColorScale: windIntensityColorScale,
        segmentedColorScale: segmentedColorScale,
        loadJson: loadJson,
        distortion: distortion,
        newAgent: newAgent,
        parse: parse,
        buildConfiguration: buildConfiguration
    };

}();
