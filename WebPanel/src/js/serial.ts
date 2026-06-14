export const MSG_TYPE = {
  CMD_FX: 0,
  CMD_SET_BR: 1
};

export interface SerialPortState {
  connected: boolean;
}

export type SerialPortDataListener = (line: string) => void;
export type SerialPortStateChangeListener = (state: SerialPortState) => void;

export class SerialPortWrapper {
  #recvBuff: string = "";
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

      };

      if (this.#port) {
        this.#port.addEventListener('connect', stChangeHandler);
        this.#port.addEventListener('disconnect', () => {
          console.error("Port connection lost");
          stChangeHandler();
        });
      } else {
        console.error("No port found");
      }

      await this.#port.open({ baudRate: 115200 });
      // console.log(await this.#port.getSignals());
      setTimeout(() => {
        this.send("node ls \n");
      }, 1000);

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

  // Todo:: move to a worker thread
  async read() {
    const decoder = new TextDecoder();

    while (this.#port?.readable) {
      this.#reader = this.#port.readable.getReader();
      try {
        while (true) {
          const { value, done } = await this.#reader.read();
          const decoder = new TextDecoder();

          this.#recvBuff += (decoder.decode(value));

          let completeLine = this.#recvBuff.slice(0, this.#recvBuff.indexOf("\n") + 1);


          while (completeLine) {
            this.#recvBuff = this.#recvBuff.split(completeLine).pop() ?? "";
            this.#listeners.forEach(l => l(completeLine.trim()));
            completeLine = this.#recvBuff.slice(0, this.#recvBuff.indexOf("\n") + 1);
          }

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

  async send(payload: string) {
    // console.debug("Sending to serial:", payload);
    if (!this.#port) return;
    let writer = null;
    try {
      if (this.#port && this.#port.writable) {
        writer = this.#port.writable.getWriter();
        const encoder = new TextEncoder();
        await writer.write(encoder.encode(payload));
        writer.releaseLock();
      }
    } catch (ex) {
      console.error(ex);
    } finally {
      writer?.releaseLock();
    }
  }
}





