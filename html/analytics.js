Raphael.fn.drawGrid = function (x, y, w, h, wv, hv, color) {
    color = color || "#000";
    var path = ["M", Math.round(x) + .5, Math.round(y) + .5, "L", Math.round(x + w) + .5, Math.round(y) + .5, Math.round(x + w) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y) + .5],
        rowHeight = h / hv,
        columnWidth = w / wv;
    for (var i = 1; i < hv; i++) {
        path = path.concat(["M", Math.round(x) + .5, Math.round(y + i * rowHeight) + .5, "H", Math.round(x + w) + .5]);
    }
    for (i = 1; i < wv; i++) {
        path = path.concat(["M", Math.round(x + i * columnWidth) + .5, Math.round(y) + .5, "V", Math.round(y + h) + .5]);
    }
    return this.path(path.join(",")).attr({stroke: color});
};

$(function () {
    $("#data").css({
        position: "absolute",
        left: "-9999em",
        top: "-9999em"
    });
});

String.prototype.printf = function (obj) {
  var useArguments = false;
  var _arguments = arguments;
  var i = -1;
  if (typeof _arguments[0] == "string") {
    useArguments = true;
  }
  if (obj instanceof Array || useArguments) {
    return this.replace(/\%s/g,
    function (a, b) {
      i++;
      if (useArguments) {
        if (typeof _arguments[i] == 'string') {
          return _arguments[i];
        }
        else {
          throw new Error("Arguments element is an invalid type");
        }
      }
      return obj[i];
    });
  }
  else {
    return this.replace(/{([^{}]*)}/g,
    function (a, b) {
      var r = obj[b];
      return typeof r === 'string' || typeof r === 'number' ? r : a;
    });
  }
};

Raphael.fn.drawGraph = function (holder, title, units_str, dataset, width, height, value_min, value_max) {
    function getAnchors(p1x, p1y, p2x, p2y, p3x, p3y) {
        var l1 = (p2x - p1x) / 2,
            l2 = (p3x - p2x) / 2,
            a = Math.atan((p2x - p1x) / Math.abs(p2y - p1y)),
            b = Math.atan((p3x - p2x) / Math.abs(p2y - p3y));
        a = p1y < p2y ? Math.PI - a : a;
        b = p3y < p2y ? Math.PI - b : b;
        var alpha = Math.PI / 2 - ((a + b) % (Math.PI * 2)) / 2,
            dx1 = l1 * Math.sin(alpha + a),
            dy1 = l1 * Math.cos(alpha + a),
            dx2 = l2 * Math.sin(alpha + b),
            dy2 = l2 * Math.cos(alpha + b);
        return {
            x1: p2x - dx1,
            y1: p2y + dy1,
            x2: p2x + dx2,
            y2: p2y + dy2
        };
    }

    var colorhue = [
      [0.6],
      [0.2],
      [0.4]
      //[210],
      //[30],
      //[120]
    ];
    var circle_colors = [
      [],
    ];
    var circle_fill_colors = [
      ["#333"],
      ["#666"],
      ["#999"],
    ];



    // Grab the data
    var labels = [],
        data = [[]];
    labels = dataset[0];
    data = dataset[1];
    //console.log(labels);
    //console.log(data);
    //console.log(data.length)
    
    // Draw
    //colorhue = .6 || Math.random(),
    var leftgutter = 75,
        bottomgutter = 50,
        topgutter = 30,
        r = Raphael(holder, width, height + 100),
        txt = {font: '12px Helvetica, Arial', fill: "#fff"},
        txt1 = {font: '10px Helvetica, Arial', fill: "#fff"},
        txt2 = {font: '12px Helvetica, Arial', fill: "#000"},
        txt_title = {font: '16px Helvetica, Arial', fill: "#fff"},
        X = (width - leftgutter) / labels.length;
    var mines = [];
    var maxes = [];
    var color = [];
    
    // Compute min/max of each dataset if multiple datasets
    // Set a different color for each dataset
    for (var j = 0; j<data.length; j++) {
        mines[j] = Math.min.apply(Math, data[j]);
        maxes[j] = Math.max.apply(Math, data[j]);
        color[j] = "hsl(" + [colorhue[j], .5, .5] + ")";
    }
    
    /*
    // Compute the min/max values of the global datas + 10% of margin
    var min = (Math.min.apply(Math, mines) - (Math.min.apply(Math, mines)*10 / 100));
    var max = (Math.max.apply(Math, maxes) + (Math.max.apply(Math, maxes)*10 / 100));
    
    var num_strs = [Math.min.apply(Math, mines).toString().length, Math.max.apply(Math, maxes).toString().length];
    var num_str_len = Math.max.apply(Math, num_strs);
    min = min.toPrecision(num_str_len);
    max = max.toPrecision(num_str_len);
    */
    
    // Compute the min/max values of the global datas if no limits were given
    if (value_min === undefined) {
        var min = (Math.min.apply(Math, mines));
    } else {
        var min = (Math.min.apply(Math, mines));
        if ( min > value_min ) {
            min = value_min;
        }
    }
    if (value_max === undefined) {
        var max = (Math.max.apply(Math, maxes));
    } else {
        var max = (Math.max.apply(Math, maxes));
        if ( max < value_max ) {
            max = value_max;
        }
    }
    
    if ( max == min ) {
        max = max + 1;
        if ( min < 0 ) {
            min = min - 1;
        }
    }
    console.log("Min : " + min + " Max : " + max);
    
    var Y = (height - bottomgutter - topgutter) / (max - min);
    console.log("H.Pixels : +" + (height - bottomgutter - topgutter));
    console.log("Y: " + Y);
    
    // Draw grid and set min/max labels
    r.drawGrid(leftgutter + X * .5 + .5, topgutter + .5, width - leftgutter - X, height - topgutter - bottomgutter, 10, 10, "#000");
    var GridMinlabel = r.set();
    var GridMaxlabel = r.set();
    GridMinlabel.push(r.text(40, topgutter + .5, max + " " + units_str).attr(txt));
    GridMaxlabel.push(r.text(40, height - bottomgutter, min + " " + units_str).attr(txt));
    
    var pathes = [];
    var blankets = [];
    var bgp = [];
    for (var j = 0; j<data.length; j++) {
      pathes[j] = r.path().attr({stroke: color[j], "stroke-width": 4, "stroke-linejoin": "round"});
      bgp[j] = r.path().attr({stroke: "none", opacity: .3, fill: color[j]});
      var label = r.set(),
          lx = 0, ly = 0,
          is_label_visible = false,
          leave_timer;
      blankets[j] = r.set();
    }

    /* Create dummy label with almost maximum text sizes so that it's correctly initialized for first display */
    label.push(r.text(60, 12, max + " " + units_str).attr(txt));
    //label.push(r.text(60, 27, "01 december 2015 00h00").attr(txt1).attr({fill: color[j]}));
    label.hide();
    var frame = r.popup(100, 100, label, "right").attr({fill: "#000", stroke: "#666", "stroke-width": 2, "fill-opacity": .7}).hide();

    r.text(width / 2, 10, title).attr(txt_title);

    var points = [];
    var bgpp = [];
    for (var i = 0, ii = labels.length; i < ii; i++) {
        for (var j = 0; j<data.length; j++) {
          var y = Math.round(height - bottomgutter - Y * (data[j][i] - min)),
              x = Math.round(leftgutter + X * (i + .5));
              
          // Only draw X-axis labels on first data set rendering
          if ( !j ) {
              var t = r.text(x, height - 6, labels[i]).attr(txt).toBack();
              t.attr({transform: "r" + 90});
              t.translate(30, 0);
          }
          if (!i) {
              console.log("Point " + j + ":" + i + " x: "+ x + " y: " + y);
              points[j] = ["M", x, y, "C", x, y];
              bgpp[j] = ["M", leftgutter + X * .5, height - bottomgutter, "L", x, y, "C", x, y];
          }
          if (i && i < ii - 1) {
              var Y0 = Math.round(height - bottomgutter - Y * (data[j][i - 1] - min)),
                  X0 = Math.round(leftgutter + X * (i - .5)),
                  Y2 = Math.round(height - bottomgutter - Y * (data[j][i + 1] - min)),
                  X2 = Math.round(leftgutter + X * (i + 1.5));
              var a = getAnchors(X0, Y0, x, y, X2, Y2);
              points[j] = points[j].concat([a.x1, a.y1, x, y, a.x2, a.y2]);
              bgpp[j] = bgpp[j].concat([a.x1, a.y1, x, y, a.x2, a.y2]);
          }
          if (i==(ii-1)) {
              bgpp[j] = bgpp[j].concat([x, y, x, y, "L", x, height - bottomgutter, "z"]);
              points[j] = points[j].concat([x, y, x, y]);
          }
          var dot = r.circle(x, y, 4).attr({fill: circle_fill_colors[j], stroke: color[j], "stroke-width": 2});
          blankets[j].push(r.rect(leftgutter + X * i, 0, X, height - bottomgutter).attr({stroke: "none", fill: "#fff", opacity: 0}));
          var rect = blankets[j][blankets[j].length - 1];
          (function (x, y, data, lbl, dot) {
              var timer, i = 0;
              rect.hover(function () {
                  clearTimeout(leave_timer);
                  var side = "right";
                  if (x + frame.getBBox().width > width) {
                      side = "left";
                  }
                  var ppp = r.popup(x, y, label, side, 1),
                      anim = Raphael.animation({
                          path: ppp.path,
                          transform: ["t", ppp.dx, ppp.dy]
                      }, 200 * is_label_visible);
                  lx = label[0].transform()[0][1] + ppp.dx;
                  ly = label[0].transform()[0][2] + ppp.dy;
                  frame.show().stop().animate(anim);
                  label[0].attr({text: data + " " + units_str}).show().stop().animateWith(frame, anim, {transform: ["t", lx, ly]}, 200 * is_label_visible);
                  //label[1].attr({text: lbl}).show().stop().animateWith(frame, anim, {transform: ["t", lx, ly]}, 200 * is_label_visible);
                  dot.attr("r", 6);
                  is_label_visible = true;
              }, function () {
                  dot.attr("r", 4);
                  leave_timer = setTimeout(function () {
                      frame.hide();
                      label[0].hide();
                      //label[1].hide();
                      is_label_visible = false;
                  }, 1);
              });
          })(x, y, data[j][i], labels[i], dot);
      }
    }
    for (var j = 0; j<data.length; j++) {
      pathes[j].attr({path: points[j]});
      bgp[j].attr({path: bgpp[j]});
    }
    frame.toFront();
    label[0].toFront();
    //label[1].toFront();
    for (var j = 0; j<data.length; j++) {
      blankets[j].toFront();
    };
};
