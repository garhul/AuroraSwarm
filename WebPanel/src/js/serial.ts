export const MSG_TYPE = {
  CMD_FX: 0,
  CMD_SET_BR: 1
};

export interface SerialPortState {
  connected: boolean;
}

export type SerialPortDataListener = (data: Uint8Array | undefined) => void;
export type SerialPortStateChangeListener = (state: SerialPortState) => void;

export class SerialPortWrapper {
  // #recvBuff: Uint8Array;
  // #sendBuff: Uint8Array;
  #port: SerialPort | null = null;
  #listeners: SerialPortDataListener[] = [];
  #stateChangeListeners: SerialPortStateChangeListener[] = [];
  #reader: ReadableStreamDefaultReader<Uint8Array<ArrayBufferLike>> | null = null;

  isConnected(): Boolean {
    return this.#port?.connected || false;
  }

  async isOpen(): Promise<boolean> {
    try {
      await this.#port?.open({ baudRate: 115200 });
      await this.#port?.close();
      return false;
    } catch (ex: unknown) {
      if ((ex as DOMException).name === "InvalidStateError") {
        return true;
      } else {
        console.error(ex);
      }
    }
    return false;
  }

  async connect(): Promise<boolean> {
    try {

      this.#port = await navigator.serial.requestPort({ filters: [{ usbVendorId: 0x303a }] });

      const stChangeHandler = () => {
        const state = { connected: this.#port?.connected || false };
        this.#stateChangeListeners.forEach(l => l(state));

        // Add init message
        if (state.connected == true) {
          setTimeout(() => {
            this.send(Uint8Array.from("node ls"));
          }, 250);
        }

      };

      if (this.#port) {
        this.#port.addEventListener('connect', stChangeHandler);
        this.#port.addEventListener('disconnect', stChangeHandler);
      } else {
        console.error("No port found");
      }

      await this.#port.open({ baudRate: 115200 });
      console.log(await this.#port.getSignals());

      this.#port?.addEventListener('disconnect', () => console.error('port disconnected'));
      this.read();
    } catch (err) {
      return false;
      console.error("Error connecting to serial port", err);
    }

    return true;
  }

  async disconnect(): Promise<boolean> {
    try {
      this.#reader?.releaseLock();
      await this.#port?.close();
      return true;
    } catch (ex) {
      console.error(ex);
      return false;
    }
  }

  onStateChange(listener: SerialPortStateChangeListener) {
    this.#stateChangeListeners.push(listener);
  }

  onMessage(listener: SerialPortDataListener) {
    this.#listeners.push(listener);
  }


  async read() {
    const decoder = new TextDecoder();
    while (this.#port?.readable) {
      this.#reader = this.#port.readable.getReader();
      try {
        while (true) { // Todo:: move to a worker thread
          const { value, done } = await this.#reader.read();
          this.#listeners.forEach(l => l(value));

          if (done) break;
        }
        // Update listeners
      } catch (err) {
        console.error(err);
      } finally {
        this.#reader.releaseLock();
      }
    }
  }

  async send(payload: Uint8Array) {
    if (!this.#port) return;

    if (this.#port && this.#port.writable) {
      const writer = this.#port.writable.getWriter();
      const encoder = new TextEncoder();
      await writer.write(encoder.encode(payload.toString()));
      writer.releaseLock();
    }
  }
}


export interface SerialTerminalWidgetSettings {
  autoScroll: boolean;
  colorLabels: boolean;
  color?: '#ff0000';
}


export class SerialTermWidget {
  #rootEl: HTMLElement;
  #port: SerialPortWrapper;
  #inputEl: HTMLInputElement | null;
  #viewPort: HTMLTextAreaElement | null;
  #settings: SerialTerminalWidgetSettings = {
    autoScroll: true,
    colorLabels: true,
  }

  constructor(rootEl: HTMLElement, port: SerialPortWrapper) {
    this.#rootEl = rootEl;
    this.#port = port;
    this.#inputEl = this.#rootEl.querySelector('.input');
    this.#viewPort = this.#rootEl.querySelector('.viewPort');

    port.onMessage((data) => {
      if (!data) return;
      const decoder = new TextDecoder();

      this.#renderNewData(decoder.decode(data));
    })

    this.#bindControls();
  }


  #bindControls() {
    const autoScrollBtn = this.#rootEl.querySelector("#autoScrollToggleBtn");
    const clearBtn = this.#rootEl.querySelector("#clearBtn");

    clearBtn?.addEventListener('click', () => {
      if (this.#viewPort)
        this.#viewPort.innerHTML = "";
    });

    autoScrollBtn?.addEventListener('click', () => {
      this.#settings.autoScroll = !this.#settings.autoScroll;
      if (!this.#settings.autoScroll) {
        autoScrollBtn.classList.remove('btn-warning');
        autoScrollBtn.classList.add('btn-secondary');
      } else {
        autoScrollBtn.classList.add('btn-warning');
        autoScrollBtn.classList.remove('btn-secondary');
      }
    });
  }

  #renderNewData(data: string) {
    if (this.#viewPort === null) return;

    this.#viewPort.innerText += data;
    if (this.#settings.autoScroll) {
      this.#viewPort.scrollTop = this.#viewPort.scrollHeight;
    }
  }

}




