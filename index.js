// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

// ========== Data loaded from formats.json ==========
// Array of plasma parameter names.
var plasmaParameterNames = [];
// Dictionary with dispersion relation names as keys
// and arrays of required plasma parameter names as values.
var requiredParameters = {};
// ===================================================

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
  for (var i = 0; i < params.length; i++)
    paramIDs.push(plasmaParameterNames.indexOf(params[i]));

  for (var i = 0; i < plasmaParameterNames.length; i++)
  {
    if (paramIDs.indexOf(i) >= 0)
      $("#parameter" + i).appendTo($("#required"));
    else
      $("#parameter" + i).appendTo($("#other"));
  }
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
  var paramFilename = $(event.target).parent().find(".paramFilename");
  paramFilename[0].textContent = filename;

  var parameter = $(event.target).closest(".parameter");
  var paramName = parameter.find(".paramName").text();
  var paramID = plasmaParameterNames.indexOf(paramName);
  var checked = parameter.find(".paramDataType").find("input:checked");
  var dataDim = 1;
  if (checked.attr("class") == "radio2D")
    dataDim = 2;

  var coordTypes = [2, -1];

  const cppmain = require("./cpp/build/Release/main");
  console.log(cppmain.load_file(filepath, dataDim, paramID, coordTypes));
}

function param0D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").show();
  param.find(".format1D").hide();
  param.find(".format2D").hide();
  param.find(".paramDataFile").hide();
}
function param1D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").hide();
  param.find(".format1D").show();
  param.find(".format2D").hide();
  param.find(".paramDataFile").show();
}
function param2D(event)
{
  var param = $(event.target).closest(".parameter");
  param.find(".paramDataValue").hide();
  param.find(".format1D").hide();
  param.find(".format2D").show();
  param.find(".paramDataFile").show();
}

function loadFormats()
{
  var fs = require("fs");
  var contents = fs.readFileSync("formats.json");
  var formats = JSON.parse(contents);

  plasmaParameterNames = formats["Plasma Parameters"];
  for (var i = 0; i < formats["Dispersion Relations"].length; i++)
  {
    var disp = formats["Dispersion Relations"][i];
    requiredParameters[disp["name"]] = disp["req"];
    for (var j = 0; j < disp["req"].length; j++)
    {
      if (plasmaParameterNames.indexOf(disp["req"][j]) == -1)
      {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR - " + disp["name"] + " requires non-existent parameter: "
          + disp["req"][j];
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

  // Generate plasma parameter divs.
  var parameter = $(".parameter");
  for (var i = 0; i < plasmaParameterNames.length; i++)
  {
    var clone = parameter.clone(true);
    clone.attr("id", "parameter" + i);
    clone.appendTo(parameter.parent());
    clone.find(".paramName").text(plasmaParameterNames[i]);
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
  }
  parameter.remove();

  // Add click callbacks.
  $(".dispButton").each(function() { $(this).click(dispSelect); });
  $(".paramFileButton").each(function() { $(this).click(fileChange); });

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
