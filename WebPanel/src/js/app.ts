import 'bootstrap/dist/css/bootstrap.min.css';
import * as bootstrap from 'bootstrap';
import '../css/style.css';
import { getControlWidget, getNavBarWidget, getNodeList, getSerialTermWidget } from "./widgets";
import { SerialPortWrapper } from './serial';


const init = () => {
  const serial = new SerialPortWrapper();

  const termRootEl = document.querySelector('#TermWidget') as HTMLElement;
  const termWidget = getSerialTermWidget(termRootEl);
  termWidget.bindToSerial(serial);

  const navBarEl = document.querySelector('.navbar') as HTMLElement;
  const navBarWidget = getNavBarWidget(navBarEl)
  navBarWidget.bindToSerial(serial);

  const nodeListEl = document.querySelector('#NodeList') as HTMLElement;
  const nodeList = getNodeList(nodeListEl);
  nodeList.bindToSerial(serial);

  const controlsEl = document.querySelector('#Controls') as HTMLElement;
  const controls = getControlWidget(controlsEl);

  nodeList.onSelectionChange((nodes) => controls.updateNodes(nodes));
  controls.bindToSerial(serial);

};

init();