// ========== Data loaded from formats.json ==========
let plasmaParams = [];
let dispRels = [];
let constants = [];
let calcParams = [];
// ===================================================

function LoadFormats(filePath)
{
  var fs = require("fs");
  var contents = fs.readFileSync(filePath);
  var formats = JSON.parse(contents);

  plasmaParams = formats["PlasmaParameters"];
  dispRels = formats["DispersionRelations"];
  constants = formats["Constants"];
  calcParams = formats["CalculatedParameters"];

  // Check all formats.json data for correctness
  // - Duplicate parameter names/aliases.
  for (let i = 0; i < plasmaParams.length; i++) {
    for (let j = i + 1; j < plasmaParams.length; j++) {
      if (plasmaParams[i]["name"] === plasmaParams[j]["name"])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate plasma parameter name - "
          + plasmaParams[i]["name"];
      if (plasmaParams[i]["alias"] === plasmaParams[j]["alias"])
        throw "ERROR (DBG): Duplicate plasma parameter alias - "
          + plasmaParams[i]["alias"];
    }
  }
  // - Duplicate dispersion relation names.
  for (let i = 0; i < dispRels.length; i++) {
    for (let j = i + 1; j < dispRels.length; j++) {
      if (dispRels[i]["name"] === dispRels[j]["name"])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate dispersion relation name - "
          + dispRels[i]["name"];
    }
  }
  // - Duplicate constant names.
  for (let i = 0; i < constants.length; i++) {
    for (let j = i + 1; j < constants.length; j++) {
      if (constants[i]["name"] === constants[j]["name"])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate constant name - " + constants[i]["name"];
    }
  }
  // - Existence of dispersion relation required parameters.
  for (let i = 0; i < dispRels.length; i++) {
    var dispRel = dispRels[i];
    for (let j = 0; j < dispRel["req"].length; j++) {
      var req = dispRel["req"][j];
      var exists = false;
      for (let p = 0; p < plasmaParams.length; p++) {
        if (req === plasmaParams[p]["alias"]) {
          exists = true;
          break;
        }
      }
      if (!exists) {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): " + dispRel["name"]
          + " requires non-existent parameter: " + dispRel["req"][j];
      }
    }
  }

  cppmain.Setup(plasmaParams, dispRels,
    formats["Constants"], formats["CalculatedParameters"]);
}
