import { SerialPortState, SerialPortWrapper } from "./serial";

/*------------------------------------ NavBar widget -------------------------------------------*/

class NavBarWidget {
  #rootEl: HTMLElement;
  #serial: SerialPortWrapper | null = null;
  #connBtn: HTMLButtonElement | null = null;
  constructor(rootEl: HTMLElement) {
    this.#rootEl = rootEl;
    this.#connBtn = document.querySelector("#connectBtn");

    this.#connBtn?.addEventListener('click', async () => {
      if (this.#serial === null) {
        throw new Error('Serial port not bound');
      }

      let result = false;

      if (await this.#serial.isOpen()) {
        result = await this.#serial.disconnect();
        if (result) {
          this.#connBtn!.innerHTML = "Connect";
          this.#update(true);
        }
      } else {
        result = await this.#serial.connect();
        if (result) {
          this.#connBtn!.innerHTML = "Disconnect";
          this.#update(false);
        }
      }

    });
  }

  bindToSerial(serial: SerialPortWrapper) {
    this.#serial = serial;

    this.#serial.onStateChange((st: SerialPortState) => {
      this.#update(st.connected);
    });
  }

  #update(connected: boolean) {
    const badge = document.querySelector("#connectedBadge");
    if (!badge) return;

    if (!connected) {
      if (this.#connBtn) this.#connBtn.innerHTML = "Disconnect";
      badge.innerHTML = "Connected";
      badge.classList.add("bg-success");
      badge.classList.remove("bg-secondary");
    } else {
      if (this.#connBtn) this.#connBtn.innerHTML = "Connect";
      // Todo - disable buttons and other inputs that go to the serial port
      badge.innerHTML = "Disconnected";
      badge.classList.add("bg-secondary");
      badge.classList.remove("bg-success");
    }
  }
}

let navBarWidget: NavBarWidget | null = null;
export function getNavBarWidget(rootEl: HTMLElement) {
  if (navBarWidget === null) {
    navBarWidget = new NavBarWidget(rootEl)
  }

  return navBarWidget;
}

/*----------------------------------- SERIAL TERM WIDGET -------------------------*/
export interface SerialTerminalWidgetSettings {
  autoScroll: boolean;
  colorLabels: boolean;
  color?: '#ff0000';
  localEcho?: boolean;
  historyLimit: number;
}

export class SerialTermWidget {
  #rootEl: HTMLElement;
  #serial: SerialPortWrapper | null = null;
  #inputEl: HTMLInputElement | null;
  #viewPort: HTMLTextAreaElement | null;
  #viewPortContent: string[] = [];
  #settings: SerialTerminalWidgetSettings = {
    autoScroll: true,
    colorLabels: true,
    localEcho: true,
    historyLimit: 1000
  }

  constructor(rootEl: HTMLElement) {
    this.#rootEl = rootEl;

    this.#inputEl = this.#rootEl.querySelector('.input');
    this.#viewPort = this.#rootEl.querySelector('.viewPort');

    this.#bindControls();
  }

  bindToSerial(serial: SerialPortWrapper) {
    this.#serial = serial;
    this.#serial.onMessage((data) => {
      if (data.length == 0) return;
      this.#renderNewData(data);
    })
  }


  #bindControls() {
    const autoScrollBtn = this.#rootEl.querySelector("#autoScrollToggleBtn");
    const autoScrollIndicator = autoScrollBtn?.querySelector('span');
    const clearBtn = this.#rootEl.querySelector("#clearBtn");

    clearBtn?.addEventListener('click', () => {
      if (this.#viewPort) {
        this.#viewPort.innerHTML = "";
        this.#viewPortContent = [];
      }
    });

    autoScrollBtn?.addEventListener('click', () => {
      this.#settings.autoScroll = !this.#settings.autoScroll;
      if (!this.#settings.autoScroll) {
        autoScrollIndicator?.classList.remove('indicator-on');
      } else {
        autoScrollIndicator?.classList.add('indicator-on');
      }
    });
  }

  #renderNewData(line: string) {
    if (this.#viewPort === null) return;

    this.#viewPortContent.push(line);
    if (this.#viewPortContent.length > this.#settings.historyLimit) {
      this.#viewPortContent = this.#viewPortContent.slice(2);
      this.#viewPortContent.unshift("... history limit reached ...");

    }

    this.#viewPort.innerText = this.#viewPortContent.join('\n');

    if (this.#settings.autoScroll) {
      this.#viewPort.scrollTop = this.#viewPort.scrollHeight;
    }
  }

}

let serialTermWidget: SerialTermWidget | null = null;
export function getSerialTermWidget(rootEl: HTMLElement): SerialTermWidget {
  if (serialTermWidget === null) {
    serialTermWidget = new SerialTermWidget(rootEl);
  }

  return serialTermWidget;
}

/*------------------------------- ESP NODES LIST WIDGET -------------------------*/
export interface NodeData {
  id: string;
  name: string;
  idx: number;
  selected: boolean;
}

export type NodeChangeListener = (nodes: Map<string, NodeData>) => void

class NodeList {
  #rootEl;
  #autoUpdate = true;
  #nodes: Map<string, NodeData> = new Map<string, NodeData>;
  #serial: SerialPortWrapper | null = null;
  #changeListeners: NodeChangeListener[] = [];
  #autoupdateInterval = 5000;


  constructor(rootEl: HTMLElement) {
    this.#rootEl = rootEl;
    this.#rootEl.querySelector('#autoUpdateToggleBtn')?.addEventListener('click', (e) => {
      this.#autoUpdate = !this.#autoUpdate;
      const indicator = (e.currentTarget as HTMLElement).querySelector('span');
      if (this.#autoUpdate) {
        indicator?.classList.add('indicator-on');
      } else {
        indicator?.classList.remove('indicator-on');
      }
    });
  }

  bindToSerial(serial: SerialPortWrapper) {
    this.#serial = serial;

    this.#serial.onMessage((data) => {
      if (!data.startsWith("[ INFO ] - [ NODE ]"))
        return;

      const rx = new RegExp(/\[ INFO \] - \[ NODE \] - SLOT (\d) - MAC \[(.*)\] \[ INFO ] -   NAME \[(.*)\]/);
      const groups = rx.exec(data);

      if (groups !== null) {
        const [_str, slot, mac, name] = groups;
        if (!this.#nodes.has(mac)) {
          this.#nodes.set(mac, { idx: parseInt(slot), id: mac, name, selected: false });
        }
      }

      this.#update();
    });

    setInterval(() => {
      if (this.#autoUpdate)
        this.#serial?.send("node ls \n");
    }, this.#autoupdateInterval);
  }

  onSelectionChange(listener: NodeChangeListener) {
    this.#changeListeners.push(listener);
  }

  #update() {
    const items = [];
    // add new elements if needed
    for (const k of this.#nodes.keys()) {
      const li = document.createElement('li');
      if (this.#nodes.get(k)?.selected)
        li.classList.add('selected')

      li.addEventListener('click', ev => {
        if (!this.#nodes.has(k)) return;

        if (this.#nodes.get(k)!.selected) {
          this.#nodes.get(k)!.selected = false;
          li.classList.remove('selected');
        } else {
          this.#nodes.get(k)!.selected = true;
          li.classList.add('selected');
        }

        this.#changeListeners.forEach(l => l(this.#nodes));
      });

      const indicator = document.createElement('span');
      indicator.classList.add('indicator');
      li.appendChild(indicator);
      const name = document.createElement('span');
      name.classList.add('name');
      name.innerHTML = this.#nodes.get(k)?.name ?? "";
      li.appendChild(name);

      items.push(li);
    };
    this.#changeListeners.forEach(l => l(this.#nodes));
    const ul = this.#rootEl.querySelector(".row > ul");

    ul!.replaceChildren(...items);
  }

}

let nodeList: NodeList | null = null;
export function getNodeList(rootEl: HTMLElement): NodeList {
  if (nodeList == null) {
    nodeList = new NodeList(rootEl);
  }

  return nodeList
}

/* ----------------------------- Controls widget ------------------------------------ */
class Controls {
  #serial: SerialPortWrapper | null = null;
  #rootEl: HTMLElement;
  #brRange: HTMLInputElement;
  #spdRange: HTMLInputElement;
  #nodes: Map<string, NodeData> = new Map<string, NodeData>;
  #updateInterval = 50;
  #updateTimer: NodeJS.Timeout | null = null;
  #sendBuffer: string = "";

  constructor(rootEl: HTMLElement) {
    this.#rootEl = rootEl;

    this.#brRange = rootEl.querySelector('#brRange') as HTMLInputElement;
    this.#spdRange = rootEl.querySelector('#spdRange') as HTMLInputElement;

    this.#bindControls();
    this.#renderSwatch();

    this.#updateTimer = setInterval(() => {
      if (this.#sendBuffer !== "") {
        if (!this.#serial) return;

        // Todo, parse and convert the buffer to a string, then
        // const payload = 

        this.#sendToSelectedNodes();
      }
    }, this.#updateInterval);

  }

  updateNodes(nodes: Map<string, NodeData>) {
    this.#nodes = nodes;
    const badge = this.#rootEl.querySelector('#SelectedNodesBadge');
    const selectedCount = nodes.entries().toArray().filter(n => n[1].selected).length;

    if (this.#nodes.size == 0 || selectedCount == 0) {
      badge!.innerHTML = `No nodes selected`;
      badge!.classList.remove('text-bg-success');
      badge!.classList.add('text-bg-warning');
      return;
    } else {
      badge!.classList.remove('text-bg-warning');
      badge!.classList.add('text-bg-success');
      badge!.innerHTML = (nodes.size == selectedCount) ? 'Selected all nodes' : `Selected ${selectedCount} nodes`;
    }
  }

  async #sendToSelectedNodes() {
    const selectedCount = this.#nodes.entries().toArray().filter(n => n[1].selected).length;
    if (selectedCount == 0) return;
    if (!this.#serial) return;

    //parse and convert the send buffer to an array of bytes, then send to the selected nodes
    const payload = `${this.#sendBuffer}`;
    this.#sendBuffer = "";

    // if (selectedCount == this.#nodes.size) {
    //send to all
    // this.#serial!.send(`node send -a ${payload}\n`);

    // } else {


    const nodeList = this.#nodes.entries().toArray().filter(n => n[1].selected).map(n => n[1].idx).join(':');


    // for (const node of this.#nodes.entries().toArray().filter(n => n[1].selected)) {

    await this.#serial!.send(`node send -i ${nodeList} ${payload}\n`);
    // };
    // }
  }

  #renderSwatch() {
    const colors = 96;
    const rows = 4;
    const swatches = [];
    for (let i = 1; i <= colors; i++) {
      const sw = document.createElement('span');
      sw.classList.add('swatch');
      const hue = i * (360 / colors);

      sw.setAttribute('data-payload', `H:HSL:${Math.ceil(i * (255 / colors))}:255:128`);
      sw.style.backgroundColor = `hsl(${hue}, 100%, 50%)`;

      sw.addEventListener('click', (el) => {
        this.#sendBuffer = `${sw.getAttribute('data-payload')}`;
      });

      swatches.push(sw);

      if (i % (colors / rows) == 0) {
        swatches.push(document.createElement('br'));
      }
    }

    const swatchContainer = this.#rootEl.querySelector('#ColorSwatches');
    swatchContainer?.replaceChildren(...swatches);

  }

  bindToSerial(serial: SerialPortWrapper) {
    this.#serial = serial;
  }

  #bindControls() {
    this.#rootEl.querySelectorAll('button[data-payload]').forEach(btn => {
      btn.addEventListener('click', () => {
        this.#sendBuffer = btn.getAttribute('data-payload') || "";
      });
    });

    this.#brRange.addEventListener('input', (e => {
      const val = (e.currentTarget as HTMLInputElement).value;
      this.#sendBuffer = `br:${val}`;
    }));

    this.#spdRange.addEventListener('input', (e => {
      const val = (e.currentTarget as HTMLInputElement).value;
      this.#sendBuffer = `spd:${val}`;
    }));
  }
}

let controls: Controls | null = null;

export function getControlWidget(rootEl: HTMLElement) {
  if (controls === null) {
    controls = new Controls(rootEl);
  }
  return controls;

}