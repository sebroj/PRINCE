// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

// ========== Data loaded from formats.json ==========
// Array of plasma parameter names.
let plasmaParameterNames = [];
// Dictionary with dispersion relation names as keys
// and arrays of required plasma parameter names as values.
let requiredParameters = {};

// TODO use these instead
let plasmaParameters = [];
let dispersionRelations = [];
// ===================================================

// Load native C++ module.
const cppmain = require("../cpp/build/Release/main");

const d3 = require("d3");

let chromeTabs = null;

/* Toggle between hiding and showing the dropdown content */
function toggleDropdown()
{
  $("#dispDropdown").toggle();
}

/* Dispersion relation has been selected. */
function dispSelect(event)
{
  // Name of dispersion relation selected.
  var dispersionName = $(event.target).text();
  // Update dropdown button text.
  $("#dropdownButton").text(dispersionName);

  // Reorder plasma parameters.
  var params = requiredParameters[dispersionName];
  var paramIDs = [];
  for (let i = 0; i < params.length; i++)
    paramIDs.push(plasmaParameterNames.indexOf(params[i]));

  for (let i = 0; i < plasmaParameters.length; i++)
  {
    if (paramIDs.indexOf(i) >= 0)
      $("#parameter" + i).appendTo($("#required"));
    else
      $("#parameter" + i).appendTo($("#other"));
  }
}

function paramPlot(event)
{
  var parameter = $(event.target).closest(".parameter");
  var paramName = parameter.find(".paramName").text();
  var paramID = plasmaParameterNames.indexOf(paramName);

  // Try to get the data
  data = cppmain.get_param_data(paramID);
  if (data == null) {
    // TODO handle user errors globally
    console.log("ERROR (USR): No data found!");
    return;
  }
  dataPairs = [];
  for (let i = 0; i < data.length / 2; i++) {
    dataPairs.push([data[i], data[i + data.length / 2]]);
  }

  // Create new tab for figure
  chromeTabs.addTab({title: paramName});
  var tabElID = paramName.replace(/ /g, "");
  var $div = $("<div>", {id: tabElID, "class": "tab-page"});
  $("body").append($div);

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
  var svg = d3.select("#" + tabElID).append("svg")
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

/* Input data file has been changed. */
function fileChange(event)
{
  // TODO undesired/annoying behavior: click "Select File" button again when
  // dialog is open causes another file upload window to pop up after.
  const dialog = require("electron").remote.dialog;
  var files = dialog.showOpenDialog({ properties: [ 'openFile' ] });
  if (files == undefined)
    return;

  var filepath = files[0];
  var filename = filepath.replace(/^.*[\\\/]/, '');

  var parameter = $(event.target).closest(".parameter");
  var paramName = parameter.find(".paramName").text();
  var paramID = plasmaParameterNames.indexOf(paramName);
  var paramFilename = parameter.find(".paramFilename");

  var checkedRadioDim = parameter.find(".paramDataType").find("input:checked");
  var dataDim = 1;
  if (checkedRadioDim.attr("class") == "radio2D")
    dataDim = 2;

  var coordTypes = [-1, -1];
  if (dataDim == 1)
  {
    var checkedRadioFormat = parameter.find(".format1D").find("input:checked");
    if (checkedRadioFormat.attr("class") == "radioX")
      coordTypes[0] = 0;
    else if (checkedRadioFormat.attr("class") == "radioY")
      coordTypes[0] = 1;
    else if (checkedRadioFormat.attr("class") == "radioZ")
      coordTypes[0] = 2;
  }
  else
  {
    var checkedRadioFormat = parameter.find(".format2D").find("input:checked");
    if (checkedRadioFormat.attr("class") == "radioXY")
    {
      coordTypes[0] = 0;
      coordTypes[1] = 1;
    }
    if (checkedRadioFormat.attr("class") == "radioXZ")
    {
      coordTypes[0] = 0;
      coordTypes[1] = 2;
    }
    if (checkedRadioFormat.attr("class") == "radioYZ")
    {
      coordTypes[0] = 1;
      coordTypes[1] = 2;
    }
  }
  var success = cppmain.load_file(filepath, dataDim, paramID, coordTypes);
  if (success)
  {
    console.log("Success!");
    paramFilename.text(filename);
  }
  else
  {
    console.log("FAILED");
    paramFilename.text("No file selected");
  }
}

// TODO factor 0-D, 1-D, and 2-D param UI elements into one div?
function param0D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").show();
  param.find(".format1D").hide();
  param.find(".format2D").hide();
  param.find(".paramDataFile").hide();
  param.find(".paramPlot").hide();
  // TODO send reset message to C++ module (clear data)
}
function param1D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").hide();
  param.find(".format1D").show();
  param.find(".format2D").hide();
  param.find(".paramDataFile").show();
  // TODO the string "No file selected" is used multiple times. factor?
  param.find(".paramFilename").text("No file selected");
  param.find(".paramPlot").show();
  // TODO send reset message to C++ module (clear data)
}
function param2D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").hide();
  param.find(".format1D").hide();
  param.find(".format2D").show();
  param.find(".paramDataFile").show();
  param.find(".paramFilename").text("No file selected");
  param.find(".paramPlot").show();
  // TODO send reset message to C++ module (clear data)
}

function loadFormats()
{
  var fs = require("fs");
  var contents = fs.readFileSync("ui/formats.json");
  var formats = JSON.parse(contents);

  plasmaParameters = formats["PlasmaParameters"];
  dispersionRelations = formats["DispersionRelations"];

  // Check all dispersion relation fields for validity
  // (e.g. references to required parameters)
  /*for (var i = 0; i < formats["DispersionRelations"].length; i++)
  {
    var dispRel = formats["DispersionRelations"][i];
    for (var j = 0; j < dispRel["req"].length; j++)
    {
      var req = dispRel["req"][j];
      var exists = false;
      for (var p = 0; p < plasmaParameters.length; p++)
      {
        if (req === plasmaParameters[p]["alias"])
        {
          exists = true;
          break;
        }
      }
      if (!exists)
      {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): " + dispRel["name"]
          + " requires non-existent parameter: " + dispRel["req"][j];
      }
    }
  }*/

  for (var i = 0; i < formats["PlasmaParameters"].length; i++)
  {
    var param = formats["PlasmaParameters"][i];
    plasmaParameterNames[i] = param["name"];
  }

  for (var i = 0; i < formats["DispersionRelations"].length; i++)
  {
    var disp = formats["DispersionRelations"][i];
    requiredParameters[disp["name"]] = disp["req"];
    for (var j = 0; j < disp["req"].length; j++)
    {
      if (plasmaParameterNames.indexOf(disp["req"][j]) == -1)
      {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): " + disp["name"]
          + " requires non-existent parameter: " + disp["req"][j];
      }
    }
  }
}

$(function() {
  console.log("Starting PRINCE");
  console.log("Node version: " + process.versions.node);
  console.log("Chrome version: " + process.versions.chrome);
  console.log("Electron version: " + process.versions.electron);

  loadFormats();
  cppmain.setup_parameters(plasmaParameters.length);

  // Initialize chrome tabs
  var chromeTabsEl = document.querySelector('.chrome-tabs');
  chromeTabs = new ChromeTabs();
  chromeTabs.init(chromeTabsEl, {
    tabOverlapDistance: 14,
    minWidth: 45,
    maxWidth: 243
  });

  $(chromeTabsEl).on("activeTabChange", function(data) {
    $(".tab-page").each(function() { $(this).hide(); });
    var tabName = $(".chrome-tab-current").find(".chrome-tab-title").text();
    tabName = tabName.replace(/ /g, "");
    $("#" + tabName).show();
  });

  chromeTabs.addTab({title: "PRINCE"});
  var tabEl = $(".chrome-tab-just-added");
  tabEl.find(".chrome-tab-close").remove();

  // Generate plasma parameter divs.
  var parameter = $(".parameter");
  for (let i = 0; i < plasmaParameters.length; i++)
  {
    var param = plasmaParameters[i];
    var clone = parameter.clone(true);
    clone.attr("id", "parameter" + i);
    clone.appendTo(parameter.parent());
    clone.find(".paramName").text(param["name"]);
    // Assign different names to each parameter's radio groups.
    clone.find(".paramDataType").find("input").each(function() {
      if ($(this).attr("type") == "radio")
        $(this).attr("name", "dataType-" + i);
    });
    clone.find(".format1D").find("input").each(function() {
      if ($(this).attr("type") == "radio")
        $(this).attr("name", "format1D-" + i);
    });
    clone.find(".format2D").find("input").each(function() {
      if ($(this).attr("type") == "radio")
        $(this).attr("name", "format2D-" + i);
    });

    // Hide data input, since the data type is still unset.
    clone.find(".paramDataValue").hide();
    clone.find(".format1D").hide();
    clone.find(".format2D").hide();
    clone.find(".paramDataFile").hide();
    clone.find(".paramPlot").hide();
  }
  parameter.remove();

  // Add click callbacks.
  $("#dropdownButton").click(toggleDropdown);

  $(".dispButton").each(function() { $(this).click(dispSelect); });
  $(".paramFileButton").each(function() { $(this).click(fileChange); });
  $(".paramPlotButton").each(function() { $(this).click(paramPlot); });

  $(".radio0D").each(function() { $(this).click(param0D); });
  $(".radio1D").each(function() { $(this).click(param1D); });
  $(".radio2D").each(function() { $(this).click(param2D); });

  // Add sneaky lose-focus for 0-D input field.
  $(".submitHidden").each(function() {
    $(this).click(function() {
      $(".paramValueField").each(function() { $(this).blur(); });
    })
  });
});

$(window).click(function(event)
{
  // Close the dropdown menu if the user clicks outside of it.
  if (!$(event.target).is("#dropdownButton"))
    $(".dropdown-content").each(function() { $(this).hide(); });
});
