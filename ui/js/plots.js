const d3 = require("d3");

function ParamPlot(event)
{
  var parameter = $(event.target).closest(".parameter");
  var paramName = parameter.find(".paramName").text();
  var paramInfo = ParamFromName(paramName);

  // Try to get the data
  paramData = cppmain.GetParamData(paramInfo["alias"]);
  if (paramData == null) {
    // TODO handle user errors globally
    console.log("ERROR (USR): No parameter data.");
    return;
  }

  // Create new tab for figure, replace existing one if necessary.
  var existingTab = chromeTabs.getTabByName(paramName);
  if (existingTab != null)
    chromeTabs.removeTab(existingTab);

  chromeTabs.addTab({title: paramName});
  var $div = $("<div>", {id: "tab-" + paramInfo["alias"], "class": "tab-page"});
  $("body").append($div);

  if (paramData["dim"] == 1) {
    // 1-D plot
    dataPairs = [];
    data = paramData["data"];
    for (let i = 0; i < data.length / 2; i++) {
      dataPairs.push([data[i], data[i + data.length / 2]]);
    }

    // set the dimensions and margins of the graph
    var margin = {top: 20, right: 20, bottom: 30, left: 100},
      width = 600 - margin.left - margin.right,
      height = 400 - margin.top - margin.bottom;

    // set the ranges
    var x = d3.scaleLinear().range([0, width]);
    var y = d3.scaleLinear().range([height, 0]);

    // append the svg obgect to the tab page
    // appends a 'group' element to 'svg'
    // moves the 'group' element to the top left margin
    var svg = d3.select("#tab-" + paramInfo["alias"]).append("svg")
      .attr("width", width + margin.left + margin.right)
      .attr("height", height + margin.top + margin.bottom)
      .append("g")
      .attr("transform",
            "translate(" + margin.left + "," + margin.top + ")");

    // Scale the range of the data
    //x.domain([0, d3.max(dataPairs, function(d) { return d[0]; })]);
    //y.domain([0, d3.max(dataPairs, function(d) { return d[1]; })]);
    x.domain(d3.extent(dataPairs, function(d) { return d[0]; }));
    extentY = d3.extent(dataPairs, function(d) { return d[1]; });
    if (extentY[0] >= 0)
      y.domain([0, extentY[1]]);
    else
      y.domain(extentY);

    // Add the scatterplot
    svg.selectAll("dot")
      .data(dataPairs)
      .enter().append("circle")
      .attr("r", 1)
      .attr("cx", function(d) { return x(d[0]); })
      .attr("cy", function(d) { return y(d[1]); });

    // Add the X Axis
    svg.append("g")
      .attr("transform", "translate(0," + height + ")")
      .call(d3.axisBottom(x));

    // Add the Y Axis
    svg.append("g")
      .call(d3.axisLeft(y));
  }
  if (paramData["dim"] == 2) {
    var width = paramData["width"];
    var height = paramData["height"];
    var values = paramData["data"];
    var maxValue = d3.max(values);

    var color = d3.scaleLinear()
      .domain([0, 255])
      .range(["steelblue", "brown"])
      .interpolate(d3.interpolateHcl);

    var imagePixels = new Uint8ClampedArray(values.length * 4);
    for (let i = 0; i < values.length; i++) {
      let byte = Math.round(values[i] / maxValue * 255);

      imagePixels[i * 4] = byte;
      imagePixels[i * 4 + 1] = byte;
      imagePixels[i * 4 + 2] = byte;
      imagePixels[i * 4 + 3] = 255;
      //var c = d3.rgb(color(byte));
      //imagePixels[i * 4] = c.r;
      //imagePixels[i * 4 + 1] = c.g;
      //imagePixels[i * 4 + 2] = c.b;
    }

    var imageData = new ImageData(imagePixels, width, height);

    var $dummyCanvas = $("<canvas>")
      .attr("width", width)
      .attr("height", height)
    $dummyCanvas[0].getContext("2d").putImageData(imageData, 0, 0);

    var canvas = d3.select("#tab-" + paramInfo["alias"]).append("canvas")
      .attr("width", width * 10)
      .attr("height", height * 10)
      .attr("style", "margin-top: 20px; margin-left: 100px;");

    var context = canvas.node().getContext("2d");
    // Flip image vertically
    context.translate(0, height * 10);
    context.scale(10, -10);
    context.drawImage($dummyCanvas[0], 0, 0);
    $dummyCanvas.remove();
  }
}
