/**
 * Created by Danko on 2016/5/24.
 */
var globes = function() {
    "use strict";

    function currentPosition() {
        var λ = _mine.floorMod(new Date().getTimezoneOffset() / 4, 360);
        return [λ, 0];
    }

    function ensureNumber(num, fallback) {
        return _.isFinite(num) || num === Infinity || num === -Infinity ? num : fallback;
    }

    function clampedBounds(bounds, view) {
        var upperLeft = bounds[0];
        var lowerRight = bounds[1];
        var x = Math.max(Math.floor(ensureNumber(upperLeft[0], 0)), 0);
        var y = Math.max(Math.floor(ensureNumber(upperLeft[1], 0)), 0);
        var xMax = Math.min(Math.ceil(ensureNumber(lowerRight[0], view.width)), view.width - 1);
        var yMax = Math.min(Math.ceil(ensureNumber(lowerRight[1], view.height)), view.height - 1);
        return {x: x, y: y, xMax: xMax, yMax: yMax, width: xMax - x + 1, height: yMax - y + 1};
    }

    function standardGlobe() {
        return {
            projection: null,

            bounds: function(view) {
                return clampedBounds(d3.geo.path().projection(this.projection).bounds({type: "Sphere"}), view);
            },

            fit: function(view) {
                var defaultProjection = this.newProjection(view);
                var bounds = d3.geo.path().projection(defaultProjection).bounds({type: "Sphere"});
                var hScale = (bounds[1][0] - bounds[0][0]) / defaultProjection.scale();
                var vScale = (bounds[1][1] - bounds[0][1]) / defaultProjection.scale();
                return Math.min(view.width / hScale, view.height / vScale) * 0.9;
            },

            center: function(view) {
                return [view.width / 2, view.height / 2];
            },

            scaleExtent: function() {
                return [25, 3000];
            },

            orientation: function(o, view) {
                var projection = this.projection, rotate = projection.rotate();
                if (_mine.isValue(o)) {
                    var parts = o.split(","), λ = +parts[0], φ = +parts[1], scale = +parts[2];
                    var extent = this.scaleExtent();
                    projection.rotate(_.isFinite(λ) && _.isFinite(φ) ?
                        [-λ, -φ, rotate[2]] :
                        this.newProjection(view).rotate());
                    projection.scale(_.isFinite(scale) ? _mine.clamp(scale, extent[0], extent[1]) : this.fit(view));
                    projection.translate(this.center(view));
                    return this;
                }
                return [(-rotate[0]).toFixed(2), (-rotate[1]).toFixed(2), Math.round(projection.scale())].join(",");
            },

            manipulator: function(startMouse, startScale) {
                var projection = this.projection;
                var sensitivity = 60 / startScale;  // seems to provide a good drag scaling factor
                var rotation = [projection.rotate()[0] / sensitivity, -projection.rotate()[1] / sensitivity];
                var original = projection.precision();
                projection.precision(original * 10);
                return {
                    move: function(mouse, scale) {
                        if (mouse) {
                            var xd = mouse[0] - startMouse[0] + rotation[0];
                            var yd = mouse[1] - startMouse[1] + rotation[1];
                            projection.rotate([xd * sensitivity, -yd * sensitivity, projection.rotate()[2]]);
                        }
                        projection.scale(scale);
                    },
                    end: function() {
                        projection.precision(original);
                    }
                };
            },


            defineMask: function(context) {
                d3.geo.path().projection(this.projection).context(context)({type: "Sphere"});
                return context;
            },

            defineMap: function(mapSvg, foregroundSvg) {
                var path = d3.geo.path().projection(this.projection);
                var defs = mapSvg.append("defs");
                defs.append("path")
                    .attr("id", "sphere")
                    .datum({type: "Sphere"})
                    .attr("d", path);
                mapSvg.append("use")
                    .attr("xlink:href", "#sphere")
                    .attr("class", "background-sphere");
                mapSvg.append("path")
                    .attr("class", "coastline");
                mapSvg.append("path")
                    .attr("class", "lakes");
                foregroundSvg.append("use")
                    .attr("xlink:href", "#sphere")
                    .attr("class", "foreground-sphere");
            }
        };
    }

    function newGlobe(source, view) {
        var result = _.extend(standardGlobe(), source);
        result.projection = result.newProjection(view);
        return result;
    }

    function orthographic() {
        return newGlobe({
            newProjection: function() {
                return d3.geo.orthographic().rotate(currentPosition()).precision(0.1).clipAngle(90);
            },
            defineMap: function(mapSvg, foregroundSvg) {
                var path = d3.geo.path().projection(this.projection);
                var defs = mapSvg.append("defs");
                var gradientFill = defs.append("radialGradient")
                    .attr("id", "orthographic-fill")
                    .attr("gradientUnits", "objectBoundingBox")
                    .attr("cx", "50%").attr("cy", "49%").attr("r", "50%");
                gradientFill.append("stop").attr("stop-color", "#303030").attr("offset", "69%");
                gradientFill.append("stop").attr("stop-color", "#202020").attr("offset", "81%");
                gradientFill.append("stop").attr("stop-color", "#101010").attr("offset", "96%");
                defs.append("path")
                    .attr("id", "sphere")
                    .datum({type: "Sphere"})
                    .attr("d", path);
                mapSvg.append("use")
                    .attr("xlink:href", "#sphere")
                    .attr("fill", "url(#orthographic-fill)");
                foregroundSvg.append("path")
                    .attr("class", "coastline");
                foregroundSvg.append("path")
                    .attr("class", "lakes");
                foregroundSvg.append("use")
                    .attr("xlink:href", "#sphere")
                    .attr("class", "foreground-sphere");
            }
        });
    }

    return d3.map({
        orthographic: orthographic
    });

}();
