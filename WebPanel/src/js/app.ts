import 'bootstrap/dist/css/bootstrap.min.css';
import * as bootstrap from 'bootstrap';
import '../css/style.css';
import { SerialPortState, SerialPortWrapper, SerialTermWidget } from "./serial";


function updateNavBarBadge(connected: boolean) {
  const badge = document.querySelector("#connectedBadge");
  if (!badge) return;

  if (connected) {
    badge.innerHTML = "Connected";
    badge.classList.add("bg-success");
    badge.classList.remove("bg-secondary");
  } else {
    // Todo - disable buttons and other inputs that go to the serial port
    badge.innerHTML = "Disconnected";
    badge.classList.add("bg-secondary");
    badge.classList.remove("bg-success");
  }


}


const init = () => {

  const serial = new SerialPortWrapper();
  const connBtn = document.querySelector("#connectBtn");

  serial.onStateChange((st: SerialPortState) => {
    if (connBtn) {
      connBtn.innerHTML = st.connected ? "Disconnect" : "Connect";
    }
    updateNavBarBadge(st.connected);
  });

  const termRootEl = document.querySelector('#TermWidget') as HTMLElement;
  console.assert(termRootEl !== null);

  const termWidget = new SerialTermWidget(termRootEl, serial);

  connBtn?.addEventListener('click', async () => {
    let result = false;
    if (await serial.isOpen()) {
      result = await serial.disconnect();
      if (result) {
        connBtn.innerHTML = "Connect";
        updateNavBarBadge(false);
      }
    } else {
      result = await serial.connect();
      if (result) {
        connBtn.innerHTML = "Disconnect";
        updateNavBarBadge(true);
      }
    }

  });
};


init();
