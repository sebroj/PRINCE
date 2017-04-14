// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

// ========== Data loaded from formats.json ==========
let plasmaParameters = [];
let dispersionRelations = [];
// ===================================================

// Load native C++ module.
const cppmain = require("../cpp/build/Release/main");

// External libraries
const d3 = require("d3");

// Chrome-style tabs global variable
let chromeTabs = null;

function parameterFromName(name)
{
  for (let i = 0; i < plasmaParameters.length; i++)
  {
    if (plasmaParameters[i]["name"] === name)
      return plasmaParameters[i];
  }
  return null;
}

function parameterFromAlias(alias)
{
  for (let i = 0; i < plasmaParameters.length; i++)
  {
    if (plasmaParameters[i]["alias"] === name)
      return plasmaParameters[i];
  }
  return null;
}

function dispersionRelationFromName(name)
{
  for (let i = 0; i < dispersionRelations.length; i++)
  {
    if (dispersionRelations[i]["name"] === name)
      return dispersionRelations[i];
  }
  return null;
}

/* Toggle between hiding and showing the dropdown content */
function toggleDropdown()
{
  $("#dispDropdown").toggle();
}

/* Dispersion relation has been selected. */
function dispSelect(event)
{
  var dispRelName = $(event.target).text();
  var dispRelInfo = dispersionRelationFromName(dispRelName);

  // Update dropdown button text.
  $("#dropdownButton").text(dispRelName);

  // Reorder plasma parameters.
  for (let i = 0; i < plasmaParameters.length; i++)
    $("#" + plasmaParameters[i]["alias"]).appendTo($("#other"));

  for (let i = 0; i < dispRelInfo["req"].length; i++)
    $("#" + dispRelInfo["req"][i]).appendTo($("#required"));
}

function paramPlot(event)
{
  var parameter = $(event.target).closest(".parameter");
  var paramName = parameter.find(".paramName").text();
  var paramInfo = parameterFromName(paramName);

  // Try to get the data
  paramData = cppmain.get_param_data(paramInfo["alias"]);
  if (paramData == null) {
    // TODO handle user errors globally
    console.log("ERROR (USR): No parameter data found!");
    return;
  }
  if (paramData["dim"] != 1)
  {
    console.log("2D plotting not supported yet.");
    return;
  }
  dataPairs = [];
  data = paramData["data"];
  for (let i = 0; i < data.length / 2; i++) {
    dataPairs.push([data[i], data[i + data.length / 2]]);
  }

  var existingTab = chromeTabs.getTabByName(paramName);
  if (existingTab != null)
    chromeTabs.removeTab(existingTab);

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
  // TODO change parameter coords in CPP when x/y/z or x,y/y,z/x,z option change

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
  var paramInfo = parameterFromName(paramName);
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
  var success = cppmain.load_file(filepath, paramInfo["alias"], dataDim, coordTypes);
  if (success)
  {
    paramFilename.text(filename);
  }
  else
  {
    console.log("FAILED");
    paramFilename.text("No file selected");
  }
}

function paramBoxReset(param)
{
  var paramName = param.find(".paramName").text();
  var paramInfo = parameterFromName(paramName);
  cppmain.clear_parameter(paramInfo["alias"]);

  param.find(".paramDataValue").hide();
  param.find(".format1D").hide();
  param.find(".format2D").hide();
  param.find(".paramDataFile").hide();
  param.find(".paramFilename").text("No file selected");
  param.find(".paramPlot").hide();
}

// TODO factor 0-D, 1-D, and 2-D param UI elements into one div?
function param0D(event)
{
  var param = $(event.target).closest(".parameter");
  paramBoxReset(param);

  param.find(".paramDataValue").show();
}
function param1D(event)
{
  var param = $(event.target).closest(".parameter");
  paramBoxReset(param);

  param.find(".format1D").show();
  param.find(".paramDataFile").show();
  param.find(".paramPlot").show();
}
function param2D(event)
{
  var param = $(event.target).closest(".parameter");
  paramBoxReset(param);

  param.find(".format2D").show();
  param.find(".paramDataFile").show();
  param.find(".paramPlot").show();
}

function loadFormats()
{
  var fs = require("fs");
  var contents = fs.readFileSync("ui/formats.json");
  var formats = JSON.parse(contents);

  plasmaParameters = formats["PlasmaParameters"];
  dispersionRelations = formats["DispersionRelations"];

  // Check all dispersion relation fields for validity
  // 1. Duplicate parameter names/aliases.
  for (let i = 0; i < plasmaParameters.length; i++)
  {
    for (let j = i + 1; j < plasmaParameters.length; j++)
    {
      if (plasmaParameters[i]["name"] === plasmaParameters[j]["name"])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate plasma parameter name - "
          + plasmaParameters[i]["name"];
      if (plasmaParameters[i]["alias"] === plasmaParameters[j]["alias"])
        throw "ERROR (DBG): Duplicate plasma parameter alias - "
          + plasmaParameters[i]["alias"];
    }
  }
  // 2. Duplicate dispersion relation names.
  for (let i = 0; i < dispersionRelations.length; i++)
  {
    for (let j = i + 1; j < dispersionRelations.length; j++)
    {
      if (dispersionRelations[i]["name"] === dispersionRelations[j]["name"])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate dispersion relation name - "
          + dispersionRelations[i]["name"];
    }
  }
  // 3. Existence of dispersion relation required parameters.
  for (let i = 0; i < dispersionRelations.length; i++)
  {
    var dispRel = dispersionRelations[i];
    for (let j = 0; j < dispRel["req"].length; j++)
    {
      var req = dispRel["req"][j];
      var exists = false;
      for (let p = 0; p < plasmaParameters.length; p++)
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
    var tabEl = data.detail.tabEl;
    var tabName = $(tabEl).find(".chrome-tab-title").text();
    var tabPageName = tabName.replace(/ /g, "");
    $(".tab-page").each(function() { $(this).hide(); });
    $("#" + tabPageName).show();
  });
  $(chromeTabsEl).on("tabAdd", function(data) {
    tabEl = data.detail.tabEl;
    var tabName = $(tabEl).find(".chrome-tab-title").text();
    if (tabName === "PRINCE")
      $(tabEl).find(".chrome-tab-close").remove();
  });
  $(chromeTabsEl).on("tabRemove", function(data) {
    tabEl = data.detail.tabEl;
    var tabName = $(tabEl).find(".chrome-tab-title").text();
    var tabPageName = tabName.replace(/ /g, "");
    $("#" + tabPageName).remove();
  });

  chromeTabs.addTab({title: "PRINCE"});

  // Generate plasma parameter divs.
  var paramPrototype = $(".parameter");
  for (let i = 0; i < plasmaParameters.length; i++)
  {
    let paramInfo = plasmaParameters[i];
    let clone = paramPrototype.clone(true);
    clone.attr("id", paramInfo["alias"]);
    clone.appendTo(paramPrototype.parent());
    clone.find(".paramName").text(paramInfo["name"]);
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
  paramPrototype.remove();

  // Add dispersion relations.
  for (let i = 0; i < dispersionRelations.length; i++)
  {
    let dispRelName = dispersionRelations[i]["name"];
    let $dispRelButton = $("<button>" + dispRelName + "</button>")
      .addClass("dispButton")
      .appendTo("#dispDropdown");
  }
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
