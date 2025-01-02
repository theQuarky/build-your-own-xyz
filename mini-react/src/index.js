// index.js
const MiniReact = {
    createElement,
    render: Reconciler.render,
    useState: HooksSystem.useState,
    useEffect: HooksSystem.useEffect
};

// For debugging
window.__MINIREACT_DEBUG__ = {
    Reconciler,
    HooksSystem,
    DOMHandler
};