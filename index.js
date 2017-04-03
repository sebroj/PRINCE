const TabGroup = require("electron-tabs");

$(function() {
  let tabGroup = new TabGroup();
  let tab = tabGroup.addTab({
    title: "PRINCE",
    src: "prince.html",
    closable: false,
    visible: true,
    active: true,
    ready: function() { console.log("PRINCE.html!"); }
  });
});
