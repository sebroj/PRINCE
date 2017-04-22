// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

// Load native C++ module.
const cppmain = require("../cpp/build/Release/main");

// Chrome-style tabs global variable
let chromeTabs = null;

function InfoFromField(data, fieldName, match)
{
  for (let i = 0; i < data.length; i++) {
    if (data[i][fieldName] === match)
      return data[i];
  }
  return null;
}

function TabIdToAlias(id)
{
  return id.replace(TAB_ID_PREFIX, "");
}

/* Toggle between hiding and showing the dropdown content */
function ToggleDropdown()
{
  $("#dispDropdown").toggle();
}

function ParamPlotButton(event)
{
  var $param = $(event.target).closest(".parameter");
  var paramInfo = InfoFromField(plasmaParams, "alias", $param.attr("id"));
  ParamPlot(paramInfo);
}

/* Dispersion relation has been selected. */
function DispSelect(event)
{
  var dispRelName = $(event.target).text();
  var dispRelInfo = InfoFromField(dispRels, "name", dispRelName);

  // Update dropdown button text.
  $("#dropdownButton").text(dispRelName);

  // Reorder plasma parameters.
  for (let i = 0; i < plasmaParams.length; i++)
    $("#" + plasmaParams[i]["alias"]).appendTo($("#other"));

  for (let i = 0; i < dispRelInfo["req"].length; i++)
    $("#" + dispRelInfo["req"][i]).appendTo($("#required"));
}

function FileChange(event)
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
  var paramInfo = InfoFromField(plasmaParams, "alias", parameter.attr("id"));
  var paramFilename = parameter.find(".paramFilename");

  var checkedRadioDim = parameter.find(".paramDataType").find("input:checked");
  var dataDim = 1;
  if (checkedRadioDim.attr("class") == "radio2D")
    dataDim = 2;

  var coordTypes = [-1, -1];
  if (dataDim == 1) {
    var checkedRadioFormat = parameter.find(".format1D").find("input:checked");
    if (checkedRadioFormat.attr("class") == "radioX")
      coordTypes[0] = 0;
    else if (checkedRadioFormat.attr("class") == "radioY")
      coordTypes[0] = 1;
    else if (checkedRadioFormat.attr("class") == "radioZ")
      coordTypes[0] = 2;
  }
  else {
    var checkedRadioFormat = parameter.find(".format2D").find("input:checked");
    if (checkedRadioFormat.attr("class") == "radioXY") {
      coordTypes[0] = 0;
      coordTypes[1] = 1;
    }
    if (checkedRadioFormat.attr("class") == "radioXZ") {
      coordTypes[0] = 0;
      coordTypes[1] = 2;
    }
    if (checkedRadioFormat.attr("class") == "radioYZ") {
      coordTypes[0] = 1;
      coordTypes[1] = 2;
    }
  }
  var success = cppmain.LoadParameter(paramInfo["alias"], filepath, dataDim, coordTypes);
  if (success) {
    paramFilename.text(filename);
  }
  else {
    console.log("FAILED");
    paramFilename.text("No file selected");
  }
}

function Load0D(paramInfo, valueStr)
{
  cppmain.LoadParameter(paramInfo["alias"], valueStr, 0, [-1, -1]);
}

// TODO don't reset param if dimension is unchanged
function ParamBoxReset($param)
{
  var paramInfo = InfoFromField(plasmaParams, "alias", $param.attr("id"));
  cppmain.ClearParameter(paramInfo["alias"]);

  $param.find(".paramDataValue").hide();
  $param.find(".format1D").hide();
  $param.find(".format2D").hide();
  $param.find(".paramDataFile").hide();
  $param.find(".paramFilename").text("No file selected");
  $param.find(".paramPlot").hide();
}

function Param0D(event)
{
  var $param = $(event.target).closest(".parameter");
  ParamBoxReset($param);

  $param.find(".paramDataValue").show();
}
function Param1D(event)
{
  var $param = $(event.target).closest(".parameter");
  ParamBoxReset($param);

  $param.find(".format1D").show();
  $param.find(".paramDataFile").show();
  $param.find(".paramPlot").show();
}
function Param2D(event)
{
  var $param = $(event.target).closest(".parameter");
  ParamBoxReset($param);

  $param.find(".format2D").show();
  $param.find(".paramDataFile").show();
  $param.find(".paramPlot").show();
}

$(function() {
  console.log("Starting PRINCE");
  console.log("Node version: " + process.versions.node);
  console.log("Chrome version: " + process.versions.chrome);
  console.log("Electron version: " + process.versions.electron);

  LoadFormats("ui/formats.json");

  // Initialize chrome tabs
  var chromeTabsEl = document.querySelector('.chrome-tabs');
  chromeTabs = new ChromeTabs();
  chromeTabs.init(chromeTabsEl, {
    tabOverlapDistance: 14,
    minWidth: 45,
    maxWidth: 243
  });

  $(chromeTabsEl).on("activeTabChange", function(data) {
    var $tabEl = $(data.detail.tabEl);
    var alias = TabIdToAlias($tabEl.attr("id"));
    $(".tab-page").each(function() { $(this).hide(); });
    $("#" + TAB_PAGE_PREFIX + alias).show();
  });
  $(chromeTabsEl).on("tabAdd", function(data) {
    var $tabEl = $(data.detail.tabEl);
    var tabName = $tabEl.find(".chrome-tab-title").text();
    if (tabName === "PRINCE")
      $tabEl.find(".chrome-tab-close").remove();
  });
  $(chromeTabsEl).on("tabRemove", function(data) {
    var $tabEl = $(data.detail.tabEl);
    var alias = TabIdToAlias($tabEl.attr("id"));
    $("#" + TAB_PAGE_PREFIX + alias).remove();
  });

  chromeTabs.addTab({title: "PRINCE"}, "prince");

  // Generate plasma parameter divs.
  var paramPrototype = $(".parameter");
  for (let i = 0; i < plasmaParams.length; i++) {
    let paramInfo = plasmaParams[i];
    let clone = paramPrototype.clone(true);
    clone.attr("id", paramInfo["alias"]);
    clone.appendTo(paramPrototype.parent());
    clone.find(".paramName").text(paramInfo["name"]
      + " (" + paramInfo["symbol"] + ")");
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
  for (let i = 0; i < dispRels.length; i++) {
    let dispRelName = dispRels[i]["name"];
    let $dispRelButton = $("<button>" + dispRelName + "</button>")
      .addClass("dispButton")
      .appendTo("#dispDropdown");
  }
  // Add click callbacks.
  $("#dropdownButton").click(ToggleDropdown);

  $(".dispButton").each(function() { $(this).click(DispSelect); });
  $(".paramFileButton").each(function() { $(this).click(FileChange); });
  $(".paramPlotButton").each(function() { $(this).click(ParamPlotButton); });

  $(".radio0D").each(function() { $(this).click(Param0D); });
  $(".radio1D").each(function() { $(this).click(Param1D); });
  $(".radio2D").each(function() { $(this).click(Param2D); });

  // Add sneaky lose-focus for 0-D input field.
  $(".submitHidden").each(function() {
    $(this).click(function() {
      var $paramValueField = $(this).parent().find(".paramValueField");
      $paramValueField.blur();
      var param = $(this).closest(".parameter");
      var paramInfo = InfoFromField(plasmaParams, "alias", param.attr("id"));
      Load0D(paramInfo, $paramValueField.val());
    });
  });

  // Add calculated parameter buttons.
  for (let i = 0; i < calcParams.length; i++) {
    var $button = $("<button>" + calcParams[i]["name"] + "</button>");
    $button.appendTo($("#solverSettings"));
    $button.click(function(event) {
      var paramName = $(event.target).text();
      var paramInfo = InfoFromField(calcParams, "name", paramName);
      var success = cppmain.CalcParameter(paramInfo["alias"],
        paramInfo["expr"], paramInfo["exprVars"]);
      if (!success) {
        console.log("FAILED");
      }
      else {
        ParamPlot(paramInfo);
      }
    });
  }
});

$(window).click(function(event)
{
  // Close the dropdown menu if the user clicks outside of it.
  if (!$(event.target).is("#dropdownButton"))
    $(".dropdown-content").each(function() { $(this).hide(); });
});
