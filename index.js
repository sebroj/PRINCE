// Jose M Rico
// March 8, 2017
// index.js
// Main script for index.html

var inputFieldNames = [
  "Electron Density",
  "Electron Temperature",
  "Neutral Density",
  "Electric Potential",
  "Radial Magnetic Field",
  "Axial Magnetic Field",
  "Azimuthal Magnetic Field",
  "Axial Ion Velocity"
];

var requiredFields = {
  "Simplified Esipchuk-Tilinin":
    ["Electron Density",
    "Neutral Density",
    "Electric Potential"],
  "Long wavelength gradient drift":
    ["Electron Density",
    "Electron Temperature",
    "Electric Potential",
    "Azimuthal Magnetic Field",
    "Axial Ion Velocity"],
  "High-frequency ExB drift":
    ["Electron Density",
    "Radial Magnetic Field",
    "Axial Magnetic Field",
    "Axial Ion Velocity"],
  "Damped warm Langmuir waves":
    ["Electron Density",
    "Electron Temperature",
    "Neutral Density"]
};

/* Toggle between hiding and showing the dropdown content */
function toggleDropdown()
{
  $("#dispDropdown").toggleClass("show");
}

/* Dispersion relation has been selected */
function dispSelect(event)
{
  // Name of dispersion relation selected.
  dispName = $(event.target).text();

  // Update dropdown button text.
  $("#dropdownButton").text(dispName);

  // Reorder input fields.
  var fields = requiredFields[dispName];
  var fieldIDs = [];
  for (var i = 0; i < fields.length; i++)
  {
    var fieldID = inputFieldNames.indexOf(fields[i]);
    fieldIDs.push(fieldID);
  }

  for (var i = 0; i < inputFieldNames.length; i++)
  {
    if (fieldIDs.indexOf(i) >= 0)
      $("#inField" + i).appendTo($("#required"));
    else
      $("#inField" + i).appendTo($("#other"));
  }
}

function fileChange(event)
{
  const dialog = require("electron").remote.dialog;
  var files = dialog.showOpenDialog({ properties: [ 'openFile' ] });
  if (files == undefined)
    return;

  var path = files[0];

  var filename = path.replace(/^.*[\\\/]/, '')
  var inFieldFileName = $(event.target).parent().find(".inFieldFileName");
  inFieldFileName[0].textContent = filename;
}

function doScaryThings()
{
  const scary = require("./cpp/build/Release/scary")
  console.log("Starting scary things...");
  console.log(scary.execute("DONE"))
  console.log("Finished scary things...");
}

$(function() {
  console.log("Starting PRINCE");
  console.log("Node version: " + process.versions.node);
  console.log("Chrome version: " + process.versions.chrome);
  console.log("Electron version: " + process.versions.electron);

  // Use input field prototype
  var inField = $(".inField");
  for (var i = 0; i < inputFieldNames.length; i++)
  {
    var clone = inField.clone(true);
    clone.attr("id", "inField" + i);
    clone.appendTo(inField.parent());
    clone.find(".inFieldName").text(inputFieldNames[i]);
  }
  inField.remove();

  $(".dispButton").each(function() {
    $(this).click(dispSelect);
  });

  $(".inFieldFileButton").each(function() {
    $(this).click(fileChange);
  });
});

window.onclick = function(event)
{
  // Close the dropdown menu if the user clicks outside of it.
  if (!$(event.target).is("#dropdownButton"))
  {
    $(".dropdown-content").each(function() {
      if ($(this).hasClass("show"))
        $(this).removeClass("show");
    });
  }
}
