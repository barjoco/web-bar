const $ = x => document.getElementById(x);
const i3workspaces_output = $("i3workspaces_output");
const new_workspace_btn = $("new_workspace_btn");
const time = $("time");
let currentWorkspace;

function updateWorkspaces(workspaces) {
  i3workspaces_output.innerHTML = "";

  for (const workspace of workspaces) {
    const btn = document.createElement("span");
    btn.innerText = workspace.name;
    btn.classList.add("workspace_btn")
    btn.onmousedown = function () { I3.msg(`workspace ${workspace.name}`) };
    if (workspace.visible) {
      currentWorkspace = workspace;
      btn.classList.add("current_workspace");
    };
    i3workspaces_output.appendChild(btn);
  }
}

window.addEventListener("load", function () {
  I3.msg("workspace number 1");
  const days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
  const d = new Date();
  const day = d.getDay();
  let date = d.getDate();
  let hr = d.getHours();
  let min = d.getMinutes();
  if (hr < 10) {
    hr = "0" + hr;
  }
  if (min < 10) {
    min = "0" + min;
  }
  if (date < 10) {
    date = "0" + date;
  }
  time.innerText = `${days[day]} ${date}, ${hr}:${min}`;
});

new_workspace_btn.onclick = () => {
  const workspaces = JSON.parse(I3.workspaces());
  I3.msg(`workspace number ${workspaces[workspaces.length-1].num+1}`);
};