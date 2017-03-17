// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

// TODO: move this data out of here
var parameterNames = [
  "Electron Density",
  "Electron Temperature",
  "Neutral Density",
  "Electric Potential",
  "Radial Magnetic Field",
  "Axial Magnetic Field",
  "Azimuthal Magnetic Field",
  "Axial Ion Velocity"
];
var requiredParams = {
  "Simplified Esipchuk-Tilinin":
    ["Electron Density",
    "Axial Ion Velocity",
    "Radial Magnetic Field",
    "Electric Potential"],
  "Long wavelength gradient drift":
    ["Electron Density",
    "Electric Potential",
    "Radial Magnetic Field"],
  "High-frequency ExB drift":
    ["Electron Density",
    "Electron Temperature",
    "Electric Potential",
    "Radial Magnetic Field"],
  "Damped warm Langmuir waves":
    ["Electron Density"]
};

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
  var params = requiredParams[dispersionName];
  var paramIDs = [];
  for (var i = 0; i < params.length; i++)
    paramIDs.push(parameterNames.indexOf(params[i]));

  for (var i = 0; i < parameterNames.length; i++)
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
  const dialog = require("electron").remote.dialog;
  var files = dialog.showOpenDialog({ properties: [ 'openFile' ] });
  if (files == undefined)
    return;

  var filepath = files[0];

  var filename = filepath.replace(/^.*[\\\/]/, '');
  var paramFilename = $(event.target).parent().find(".paramFilename");
  paramFilename[0].textContent = filename;

  var paramDataType = $(event.target).closest(".parameter").find(".paramDataType");
  var checked = paramDataType.find("input:checked");
  var dataDim = 1;
  if (checked.attr("class") == "radio2D")
    dataDim = 2;

  const cppmain = require("./cpp/build/Release/main");
  console.log(cppmain.load_file(filepath, dataDim));
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

function doScaryThings()
{
  //const scary = require("./cpp/build/Release/scary");
  //console.log("Starting scary things...");
  //console.log(scary.execute("DONE"));
  //console.log("Finished scary things...");
}

$(function() {
  console.log("Starting PRINCE");
  console.log("Node version: " + process.versions.node);
  console.log("Chrome version: " + process.versions.chrome);
  console.log("Electron version: " + process.versions.electron);

  // Generate plasma parameter divs.
  var parameter = $(".parameter");
  for (var i = 0; i < parameterNames.length; i++)
  {
    var clone = parameter.clone(true);
    clone.attr("id", "parameter" + i);
    clone.appendTo(parameter.parent());
    clone.find(".paramName").text(parameterNames[i]);
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
