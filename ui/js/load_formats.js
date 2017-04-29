// ========== Data loaded from formats.json ==========
let plasmaParams = [];
let dispRels = [];
let constants = [];
let calcParams = [];
// ===================================================

function CheckNoDuplicates(data, fieldStr)
{
  for (let i = 0; i < data.length; i++) {
    for (let j = i + 1; j < data.length; j++) {
      if (data[i][fieldStr] === data[j][fieldStr])
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): Duplicate " + fieldStr + " - " + data[i][fieldStr];
    }
  }
}

function LoadFormats(filePath)
{
  var fs = require("fs");
  var contents = fs.readFileSync(filePath);
  var formats = JSON.parse(contents);

  plasmaParams = formats["PlasmaParameters"];
  dispRels = formats["DispersionRelations"];
  constants = formats["Constants"];
  var otherParams = formats["OtherParameters"];
  calcParams = formats["CalculatedParameters"];

  // Check all formats.json data for correctness
  // - Duplicate names/aliases within each category.
  CheckNoDuplicates(plasmaParams, "name");
  CheckNoDuplicates(plasmaParams, "alias");
  CheckNoDuplicates(dispRels, "name");
  CheckNoDuplicates(constants, "name");
  CheckNoDuplicates(constants, "alias");
  CheckNoDuplicates(calcParams, "name");
  CheckNoDuplicates(calcParams, "alias");
  // - Existence of dispersion relation required parameters.
  for (let i = 0; i < dispRels.length; i++) {
    let dispRel = dispRels[i];
    for (let j = 0; j < dispRel["req"].length; j++) {
      let req = dispRel["req"][j];
      let exists = false;
      for (let p = 0; p < plasmaParams.length; p++) {
        if (req === plasmaParams[p]["alias"]) {
          exists = true;
          break;
        }
      }
      if (!exists) {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): " + dispRel["name"]
          + " requires non-existent parameter: " + req;
      }
    }
  }
  // - Existence of calculated parameter expression variables.
  for (let i = 0; i < calcParams.length; i++) {
    let calcParam = calcParams[i];
    for (let j = 0; j < calcParam["exprVars"].length; j++) {
      let exprVar = calcParam["exprVars"][j];
      let exists = false;
      for (let k = 0; k < plasmaParams.length; k++) {
        if (exprVar === plasmaParams[k]["alias"]) {
          exists = true;
          break;
        }
      }
      for (let k = 0; k < constants.length; k++) {
        if (exprVar === constants[k]["alias"]) {
          exists = true;
          break;
        }
      }
      for (let k = 0; k < otherParams.length; k++) {
        if (exprVar === otherParams[k]["alias"]) {
          exists = true;
          break;
        }
      }
      if (!exists) {
        // TODO make a DEBUG_error type thing for this
        throw "ERROR (DBG): " + calcParam["name"]
          + " expression uses non-existent alias: " + exprVar;
      }
    }
  }

  cppmain.Setup(plasmaParams, dispRels,
    formats["Constants"], formats["CalculatedParameters"]);
}
