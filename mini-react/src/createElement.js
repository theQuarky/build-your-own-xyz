/**
 * MiniReact Virtual DOM Creation Module
 * Handles the creation of virtual DOM elements and text nodes
 * @module createElement
 */
const createElement = (function () {
    const VALID_TYPES = new Set(['string', 'function']);

    /**
     * Validates element type and props
     * @param {string|function} elementType - Element type to validate
     * @param {Object} props - Props to validate
     * @throws {Error} If type or props are invalid
     */
    function validateElement(elementType, props) {
        const typeOf = typeof elementType;
        if (!VALID_TYPES.has(typeOf)) {
            throw new Error(`Invalid element type: ${typeOf}. Must be string or function`);
        }

        if (props !== null && typeof props !== 'object') {
            throw new Error('Props must be an object or null');
        }
    }

    /**
     * Sanitizes child content
     * @param {*} child - Child content to sanitize
     * @returns {string} Sanitized content
     */
    function sanitizeContent(child) {
        if (child === null || child === undefined) {
            return '';
        }
        return String(child);
    }

    /**
     * Creates a virtual DOM element
     * @param {string|function} elementType - Element type (e.g., 'div') or component function
     * @param {Object|null} elementProps - Element properties
     * @param {...*} children - Child elements or content
     * @returns {VirtualElement} Virtual DOM element
     */
    function createElement(elementType, elementProps, ...children) {
        try {
            validateElement(elementType, elementProps);

            // Optimize props handling
            const props = elementProps || {};
            const flattenedChildren = [];
            
            // Optimize children flattening
            const flattenChildren = (items) => {
                items.forEach(item => {
                    if (Array.isArray(item)) {
                        flattenChildren(item);
                    } else {
                        flattenedChildren.push(
                            item && item.type 
                                ? item // Already a virtual element
                                : createTextElement(item) // Create text element
                        );
                    }
                });
            };

            flattenChildren(children);

            return {
                type: elementType,
                props: {
                    ...props,
                    children: flattenedChildren
                }
            };
        } catch (error) {
            console.error('Error creating element:', error);
            // Return a fallback element in case of error
            return createErrorElement(error.message);
        }
    }

    /**
     * Creates a virtual text element
     * @param {*} content - Content to convert to text
     * @returns {VirtualElement} Virtual text element
     */
    function createTextElement(content) {
        return {
            type: "TEXT_ELEMENT",
            props: {
                nodeValue: sanitizeContent(content),
                children: []
            }
        };
    }

    /**
     * Creates an error element for fallback rendering
     * @param {string} message - Error message
     * @returns {VirtualElement} Error element
     * @private
     */
    function createErrorElement(message) {
        return {
            type: "span",
            props: {
                className: "error",
                style: "color: red; font-family: monospace;",
                children: [createTextElement(`🚫 Render Error: ${message}`)]
            }
        };
    }

    // Cache for common elements (optimization)
    const elementCache = new Map();
    const MAX_CACHE_SIZE = 1000;

    /**
     * Creates or retrieves a cached element
     * @param {string} key - Cache key
     * @param {Function} creator - Element creator function
     * @returns {VirtualElement} Cached or new element
     */
    function getCachedElement(key, creator) {
        if (elementCache.has(key)) {
            return elementCache.get(key);
        }

        const element = creator();

        if (elementCache.size >= MAX_CACHE_SIZE) {
            const firstKey = elementCache.keys().next().value;
            elementCache.delete(firstKey);
        }

        elementCache.set(key, element);
        return element;
    }

    return createElement;
})();