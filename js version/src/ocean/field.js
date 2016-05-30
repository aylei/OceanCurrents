/**
 * Created by Danko on 2016/5/24.
 */
var field = function() {
    "use strict";

    var OSCAR_PATH = "/data/oscar";
    var catalogs = {
        oscar: _mine.loadJson([OSCAR_PATH, "catalog.json"].join("/"))
    };

    function buildField(overrides) {
        return _.extend({
            description: "",
            paths: [],
            date: null,
            load: function(cancel) {
                var me = this;
                return when.map(this.paths, _mine.loadJson).then(function(files) {
                    return cancel.requested ? null : _.extend(me, buildGrid(me.builder.apply(me, files)));
                });
            }
        }, overrides);
    }

    var FACTORIES = {
        "currents": {
            matches: _.matches({param: "ocean", surface: "surface", level: "currents"}),
            create: function() {
                return buildField({
                    field: "vector",
                    type: "currents",
                    paths: [OSCAR_PATH + "/20140131-surface-currents-oscar-0.33.json"],
                    builder: function(file) {
                        var uData = file[0].data, vData = file[1].data;
                        return {
                            header: file[0].header,
                            interpolate: bilinearInterpolateVector,
                            data: function(i) {
                                var u = uData[i], v = vData[i];
                                return _mine.isValue(u) && _mine.isValue(v) ? [u, v] : null;
                            }
                        }
                    },
                    scale: {
                        bounds: [0, 1.5],
                        gradient: _mine.segmentedColorScale([
                            [0, [13, 68, 127]],
                            [0.15, [13, 68, 127]],
                            [0.4, [13, 68, 127]],
                            [0.65, [13, 68, 127]],
                            [1.0, [13, 68, 127]],
                            [1.5, [13, 68, 127]]
                        ])
                    },
                    particles: {velocityScale: 1/4400, maxIntensity: 0.7}
                });
            }
        },
    };

    function bilinearInterpolateVector(x, y, g00, g10, g01, g11) {
        var rx = (1 - x);
        var ry = (1 - y);
        var a = rx * ry,  b = x * ry,  c = rx * y,  d = x * y;
        var u = g00[0] * a + g10[0] * b + g01[0] * c + g11[0] * d;
        var v = g00[1] * a + g10[1] * b + g01[1] * c + g11[1] * d;
        return [u, v, Math.sqrt(u * u + v * v)];
    }

    function buildGrid(builder) {
        var header = builder.header;
        var λ0 = header.lo1, φ0 = header.la1;
        var Δλ = header.dx, Δφ = header.dy;
        var ni = header.nx, nj = header.ny;
        var date = new Date(header.refTime);
        date.setHours(date.getHours() + header.forecastTime);

        var grid = [], p = 0;
        var isContinuous = Math.floor(ni * Δλ) >= 360;
        for (var j = 0; j < nj; j++) {
            var row = [];
            for (var i = 0; i < ni; i++, p++) {
                row[i] = builder.data(p);
            }
            if (isContinuous) {
                row.push(row[0]);
            }
            grid[j] = row;
        }

        function interpolate(λ, φ) {
            var i = _mine.floorMod(λ - λ0, 360) / Δλ;
            var j = (φ0 - φ) / Δφ;

            var fi = Math.floor(i), ci = fi + 1;
            var fj = Math.floor(j), cj = fj + 1;

            var row;
            if ((row = grid[fj])) {
                var g00 = row[fi];
                var g10 = row[ci];
                if (!_mine.isValue(g00)) {
                    g00 = [0, 0, 0];
                }
                if (!_mine.isValue(g10)) {
                    g10 = [0, 0, 0];
                }
                if (_mine.isValue(g00) && _mine.isValue(g10) && (row = grid[cj])) {
                    var g01 = row[fi];
                    var g11 = row[ci];
                    if (!_mine.isValue(g01)) {
                        g01 = [0, 0, 0];
                    }
                    if (!_mine.isValue(g11)) {
                        g11 = [0, 0, 0];
                    }
                    if (_mine.isValue(g01) && _mine.isValue(g11)) {
                        return builder.interpolate(i - fi, j - fj, g00, g10, g01, g11);
                    }
                }
            }
            return null;
        }

        return {
            interpolate: interpolate
        };
    }

    function productsFor(attributes) {
        var attr = _.clone(attributes), results = [];
        _.values(FACTORIES).forEach(function(factory) {
            if (factory.matches(attr)) {
                results.push(factory.create());
            }
        });
        return results.filter(_mine.isValue);
    }

    return {
        overlayTypes: d3.set(_.keys(FACTORIES)),
        productsFor: productsFor
    };

}();
