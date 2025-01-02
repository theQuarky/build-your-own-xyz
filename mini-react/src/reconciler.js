// reconciler.js
/**
 * Reconciler Module
 * Manages the fiber tree, work scheduling, and DOM updates
 * @module Reconciler
 */
const Reconciler = (function () {
    /** @type {Fiber|null} Current work unit being processed */
    let nextUnitOfWork = null;
    /** @type {Fiber|null} Current root of the fiber tree */
    let currentRoot = null;
    /** @type {Fiber|null} Work in progress root */
    let workInProgressRoot = null;
    /** @type {Array<Fiber>} Fibers to be deleted */
    let deletions = [];

    // Initialize HooksSystem with update scheduler
    HooksSystem.setUpdateScheduler((fiber) => {
        if (!currentRoot) return;

        workInProgressRoot = {
            ...createFiber(currentRoot.type, currentRoot.props, currentRoot.dom),
            alternate: currentRoot
        };

        workInProgressRoot.hooks = currentRoot.hooks;
        nextUnitOfWork = workInProgressRoot;
        deletions = [];
    });

    /**
     * Creates a new Fiber node
     * @param {string|function} type - Element type
     * @param {Object} props - Element properties
     * @param {HTMLElement|Text|null} dom - DOM node
     * @returns {Fiber} New fiber node
     */
    function createFiber(type, props, dom = null) {
        return {
            type,
            props,
            dom,
            parent: null,
            child: null,
            sibling: null,
            alternate: null,
            effectTag: "PLACEMENT",
            hooks: []
        };
    }

    /**
     * Initiates render of an element into a container
     * @param {VirtualElement} element - Element to render
     * @param {HTMLElement} container - Container element
     */
    function render(element, container) {
        workInProgressRoot = createFiber("ROOT_NODE", { children: [element] }, container);
        workInProgressRoot.alternate = currentRoot;
        nextUnitOfWork = workInProgressRoot;
        deletions = [];
    }

    /**
     * Reconciles children of a fiber with new elements
     * @param {Fiber} parentFiber - Parent fiber
     * @param {Array<VirtualElement>} elements - New elements
     */
    function reconcileChildren(parentFiber, elements) {
        let oldFiber = parentFiber.alternate?.child;
        let prevSibling = null;
        let index = 0;

        while (index < elements.length || oldFiber) {
            const element = elements[index];
            let newFiber = null;

            // Compare oldFiber with element
            const sameType = oldFiber && element && element.type === oldFiber.type;

            if (sameType) {
                // Update existing fiber
                newFiber = {
                    type: oldFiber.type,
                    props: element.props,
                    dom: oldFiber.dom,
                    parent: parentFiber,
                    alternate: oldFiber,
                    effectTag: "UPDATE",
                    hooks: oldFiber.hooks || []
                };
            }

            if (element && !sameType) {
                // Create new fiber
                newFiber = {
                    type: element.type,
                    props: element.props,
                    dom: null,
                    parent: parentFiber,
                    alternate: null,
                    effectTag: "PLACEMENT",
                    hooks: []
                };
            }

            if (oldFiber && !sameType) {
                // Delete old fiber
                oldFiber.effectTag = "DELETION";
                deletions.push(oldFiber);
            }

            if (oldFiber) {
                oldFiber = oldFiber.sibling;
            }

            if (index === 0) {
                parentFiber.child = newFiber;
            } else if (prevSibling && element) {
                prevSibling.sibling = newFiber;
            }

            prevSibling = newFiber;
            index++;
        }
    }

    /**
     * Updates a function component
     * @param {Fiber} fiber - Function component fiber
     */
    function updateFunctionComponent(fiber) {
        // Set current fiber in HooksSystem
        HooksSystem.setCurrentFiber(fiber);
        fiber.hooks = [];

        try {
            const children = [fiber.type(fiber.props)];
            reconcileChildren(fiber, children);
        } catch (error) {
            console.error('Error in function component:', error);
            const errorElement = {
                type: 'div',
                props: {
                    className: 'error',
                    children: [{
                        type: 'TEXT_ELEMENT',
                        props: {
                            nodeValue: `Error: ${error.message}`,
                            children: []
                        }
                    }]
                }
            };
            reconcileChildren(fiber, [errorElement]);
        }
    }

    /**
     * Updates a host (DOM) component
     * @param {Fiber} fiber - Host component fiber
     */
    function updateHostComponent(fiber) {
        if (!fiber.dom) {
            fiber.dom = DOMHandler.createDOMNode(fiber);
        }
        reconcileChildren(fiber, fiber.props.children);
    }

    /**
     * Performs work on a fiber unit
     * @param {Fiber} fiber - Current fiber being processed
     * @returns {Fiber|null} Next unit of work
     */
    function performUnitOfWork(fiber) {
        try {
            const isFunctionComponent = fiber.type instanceof Function;

            if (isFunctionComponent) {
                updateFunctionComponent(fiber);
            } else {
                updateHostComponent(fiber);
            }

            // Return next unit of work - first child
            if (fiber.child) {
                return fiber.child;
            }

            // If no children, try siblings
            let nextFiber = fiber;
            while (nextFiber) {
                if (nextFiber.sibling) {
                    return nextFiber.sibling;
                }
                nextFiber = nextFiber.parent;
            }

            return null;
        } catch (error) {
            console.error('Error performing unit of work:', error);
            return null;
        }
    }

    /**
     * Commits a deletion to the DOM
     * @param {Fiber} fiber - Fiber to delete
     * @param {HTMLElement} domParent - Parent DOM node
     */
    function commitDeletion(fiber, domParent) {
        try {
            if (fiber.dom) {
                domParent.removeChild(fiber.dom);
                DOMHandler.cleanup(fiber.dom);
            } else {
                commitDeletion(fiber.child, domParent);
            }
        } catch (error) {
            console.error('Error during deletion:', error);
        }
    }

    /**
     * Commits a single fiber's changes to the DOM
     * @param {Fiber} fiber - Fiber to commit
     */
    function commitWork(fiber) {
        if (!fiber) return;

        try {
            let domParentFiber = fiber.parent;
            while (domParentFiber && !domParentFiber.dom) {
                domParentFiber = domParentFiber.parent;
            }

            if (domParentFiber && fiber.effectTag) {
                const domParent = domParentFiber.dom;

                switch (fiber.effectTag) {
                    case "PLACEMENT":
                        if (fiber.dom) {
                            domParent.appendChild(fiber.dom);
                        }
                        break;
                    case "UPDATE":
                        if (fiber.dom) {
                            DOMHandler.updateDOMNode(
                                fiber.dom,
                                fiber.alternate?.props || {},
                                fiber.props
                            );
                        }
                        break;
                    case "DELETION":
                        commitDeletion(fiber, domParent);
                        break;
                }
            }

            // Commit children and siblings
            commitWork(fiber.child);
            commitWork(fiber.sibling);
        } catch (error) {
            console.error('Error committing work:', error);
        }
    }

    /**
     * Commits all changes to the DOM
     */
    function commitRoot() {
        try {
            deletions.forEach(fiber => commitWork(fiber));
            commitWork(workInProgressRoot.child);
            currentRoot = workInProgressRoot;
            workInProgressRoot = null;
        } catch (error) {
            console.error('Error committing root:', error);
        }
    }

    /**
     * Main work loop for fiber processing
     * @param {IdleDeadline} deadline - Deadline from requestIdleCallback
     */
    function workLoop(deadline) {
        let shouldYield = false;

        try {
            while (nextUnitOfWork && !shouldYield) {
                nextUnitOfWork = performUnitOfWork(nextUnitOfWork);
                shouldYield = deadline.timeRemaining() < 1;
            }

            if (!nextUnitOfWork && workInProgressRoot) {
                commitRoot();
            }
        } catch (error) {
            console.error('Error in work loop:', error);
            nextUnitOfWork = null;
        }

        requestIdleCallback(workLoop);
    }

    // Start the work loop
    requestIdleCallback(workLoop);

    // Public API
    return {
        render,
        getCurrentRoot: () => currentRoot
    };
})();