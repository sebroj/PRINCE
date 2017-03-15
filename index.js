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

var parameters = {
  "Simplified Esipchuk-Tilinin": ["Electron Density"]
};

/* Toggle between hiding and showing the dropdown content */
function toggleDropdown()
{
  document.getElementById("dispDropdown").classList.toggle("show");
}

/* Dispersion relation has been selected */
function dispSelect(event)
{
  // Name of dispersion relation selected.
  dispName = event.target.textContent;

  // Update dropdown button text.
  dropdownButton = document.getElementById("dropdownButton");
  dropdownButton.textContent = dispName;

  // TODO other things should happen here.
  //  (input field layout re-ordering)
}

function fileChange(event)
{
  const dialog = require("electron").remote.dialog;
  var files = dialog.showOpenDialog({ properties: [ 'openFile' ] });
  if (files == undefined)
    return;

  var path = files[0];

  var filename = path.replace(/^.*[\\\/]/, '')
  var inFieldFileName = $(event.target.parentNode).find(".inFieldFileName");
  inFieldFileName[0].textContent = filename;
}

function doScaryThings()
{
  const scary = require("./cpp/build/Release/scary")
  console.log("Starting scary things...");
  console.log(scary.execute("DONE"))
  console.log("Finished scary things...");
}

$(function()
{
  console.log("Starting PRINCE");
  console.log("Node version: " + process.versions.node);
  console.log("Chrome version: " + process.versions.chrome);
  console.log("Electron version: " + process.versions.electron);

  // Use input field prototype
  var inField = $(".inField")[0];
  for (var i = 0; i < inputFieldNames.length; i++)
  {
    var clone = inField.cloneNode(true);
    clone.id = "inField" + i
    inField.parentNode.appendChild(clone);
    $(clone).find(".inFieldName")[0].textContent = inputFieldNames[i];
  }
  inField.parentNode.removeChild(inField);

  // Add dispersion relation selection callback to buttons.
  var dispButtons = $(".dispButton");
  for (var i = 0; i < dispButtons.length; i++)
    dispButtons[i].onclick = dispSelect;

  var inFieldFileButtons = $(".inFieldFileButton");
  for (var i = 0; i < inFieldFileButtons.length; i++)
    inFieldFileButtons[i].onclick = fileChange;
});

window.onclick = function(event)
{
  // Close the dropdown menu if the user clicks outside of it.
  if (!event.target.matches('#dropdownButton'))
  {
    var dropdowns = $(".dropdown-content");
    for (var i = 0; i < dropdowns.length; i++)
    {
      var openDropdown = dropdowns[i];
      if (openDropdown.classList.contains('show'))
        openDropdown.classList.remove('show');
    }
  }
}
