/**
 * DOM Operations Module
 * Manages direct interactions with browser DOM
 * @module DOMHandler
 */
const DOMHandler = (function () {
    // Event listener optimization
    const eventListenerMap = new WeakMap();

    /**
     * Safely creates a DOM node
     * @param {Fiber} fiberNode - Fiber node containing element info
     * @returns {HTMLElement|Text} Real DOM node
     * @throws {Error} If node creation fails
     */
    function createDOMNode(fiberNode) {
        try {
            const domNode = fiberNode.type === "TEXT_ELEMENT"
                ? document.createTextNode("")
                : document.createElement(fiberNode.type);

            updateDOMNode(domNode, {}, fiberNode.props);
            return domNode;
        } catch (error) {
            console.error('Error creating DOM node:', error);
            return createErrorNode(error.message);
        }
    }

    /**
     * Creates an error display node
     * @param {string} message - Error message
     * @returns {HTMLElement} Error node
     * @private
     */
    function createErrorNode(message) {
        const node = document.createElement('span');
        node.style.color = 'red';
        node.style.fontFamily = 'monospace';
        node.textContent = `🚫 DOM Error: ${message}`;
        return node;
    }

    // Optimized property checkers
    const propertyTypeMap = {
        EVENT: Symbol('event'),
        ATTRIBUTE: Symbol('attribute'),
        STYLE: Symbol('style'),
        OTHER: Symbol('other')
    };

    const getPropertyType = (name) => {
        if (name.startsWith('on')) return propertyTypeMap.EVENT;
        if (name === 'className' || name === 'id') return propertyTypeMap.ATTRIBUTE;
        if (name === 'style') return propertyTypeMap.STYLE;
        return propertyTypeMap.OTHER;
    };

    /**
     * Updates a DOM node's properties and event listeners
     * @param {HTMLElement|Text} domNode - DOM node to update
     * @param {Object} prevProps - Previous properties
     * @param {Object} nextProps - New properties
     */
    function updateDOMNode(domNode, prevProps, nextProps) {
        try {
            // Handle Events
            updateEventListeners(domNode, prevProps, nextProps);

            // Handle Styles
            if (prevProps.style || nextProps.style) {
                updateStyles(domNode, prevProps.style || {}, nextProps.style || {});
            }

            // Handle other properties
            updateProperties(domNode, prevProps, nextProps);
        } catch (error) {
            console.error('Error updating DOM node:', error);
            // Add visual indicator of error
            domNode.dataset.error = error.message;
        }
    }

    /**
     * Updates event listeners with improved memory management
     * @private
     */
    function updateEventListeners(domNode, prevProps, nextProps) {
        // Get or create listener map for this node
        let nodeListeners = eventListenerMap.get(domNode);
        if (!nodeListeners) {
            nodeListeners = new Map();
            eventListenerMap.set(domNode, nodeListeners);
        }

        // Remove old listeners
        Object.keys(prevProps)
            .filter(key => key.startsWith('on'))
            .forEach(name => {
                if (!(name in nextProps) || prevProps[name] !== nextProps[name]) {
                    const eventType = name.toLowerCase().substring(2);
                    const existingListener = nodeListeners.get(eventType);
                    if (existingListener) {
                        domNode.removeEventListener(eventType, existingListener);
                        nodeListeners.delete(eventType);
                    }
                }
            });

        // Add new listeners
        Object.keys(nextProps)
            .filter(key => key.startsWith('on'))
            .forEach(name => {
                const eventType = name.toLowerCase().substring(2);
                const existingListener = nodeListeners.get(eventType);
                const newListener = nextProps[name];

                if (!existingListener || existingListener !== newListener) {
                    if (existingListener) {
                        domNode.removeEventListener(eventType, existingListener);
                    }
                    domNode.addEventListener(eventType, newListener);
                    nodeListeners.set(eventType, newListener);
                }
            });
    }

    /**
     * Updates style properties efficiently
     * @private
     */
    function updateStyles(domNode, prevStyles, nextStyles) {
        // Remove old styles
        Object.keys(prevStyles).forEach(style => {
            if (!(style in nextStyles)) {
                domNode.style[style] = '';
            }
        });

        // Set new styles
        Object.keys(nextStyles).forEach(style => {
            if (prevStyles[style] !== nextStyles[style]) {
                domNode.style[style] = nextStyles[style];
            }
        });
    }

    /**
     * Updates DOM properties efficiently
     * @private
     */
    function updateProperties(domNode, prevProps, nextProps) {
        // Remove old properties
        Object.keys(prevProps).forEach(name => {
            const propType = getPropertyType(name);
            if (propType === propertyTypeMap.ATTRIBUTE && !(name in nextProps)) {
                domNode.removeAttribute(name === 'className' ? 'class' : name);
            }
        });

        // Set new properties
        Object.keys(nextProps).forEach(name => {
            if (name === 'children' || name.startsWith('on')) return;

            const propType = getPropertyType(name);
            const prevValue = prevProps[name];
            const nextValue = nextProps[name];

            if (prevValue !== nextValue) {
                switch (propType) {
                    case propertyTypeMap.ATTRIBUTE:
                        const attributeName = name === 'className' ? 'class' : name;
                        if (nextValue) {
                            domNode.setAttribute(attributeName, nextValue);
                        } else {
                            domNode.removeAttribute(attributeName);
                        }
                        break;
                    case propertyTypeMap.OTHER:
                        domNode[name] = nextValue;
                        break;
                }
            }
        });
    }

    // Memory cleanup function
    function cleanup(node) {
        eventListenerMap.delete(node);
        Array.from(node.children).forEach(cleanup);
    }

    return {
        createDOMNode,
        updateDOMNode,
        cleanup
    };
})();