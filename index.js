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
  var path = dialog.showOpenDialog({ properties: [ 'openFile' ] })[0];

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
  console.log("Node version: " + process.versions.node)
  console.log("Chrome version: " + process.versions.chrome)
  console.log("Electron version: " + process.versions.electron)

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
