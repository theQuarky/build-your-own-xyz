// hooks.js
/**
 * Hooks System Module
 * Manages state and side effects for function components
 * @module HooksSystem
 */
const HooksSystem = (function () {
    let hookIndex = 0;
    let currentFiber = null;

    /**
     * Sets the current fiber being processed
     * @param {Fiber} fiber - The fiber being processed
     */
    function setCurrentFiber(fiber) {
        currentFiber = fiber;
        hookIndex = 0;
    }

    /**
     * Creates or retrieves a state hook
     * @param {*} initialState - Initial state value
     * @returns {[*, Function]} State value and setter function
     */
    function useState(initialState) {
        if (!currentFiber) {
            throw new Error('Hooks can only be called inside function components');
        }

        const oldHook = currentFiber.alternate?.hooks?.[hookIndex];

        // Initialize hook with previous or initial state
        const hook = {
            state: oldHook ? oldHook.state : initialState,
            queue: []
        };

        // Process pending actions
        if (oldHook) {
            const actions = oldHook.queue;
            actions.forEach(action => {
                hook.state = typeof action === "function"
                    ? action(hook.state)
                    : action;
            });
        }

        const setState = action => {
            hook.queue.push(action);
            // Request update through callback
            requestUpdate(currentFiber);
        };

        // Initialize hooks array if needed
        if (!currentFiber.hooks) {
            currentFiber.hooks = [];
        }

        currentFiber.hooks.push(hook);
        hookIndex++;

        return [hook.state, setState];
    }

    let requestUpdate = () => {
        console.warn('Update scheduler not set');
    };

    /**
     * Sets the update scheduler function
     * @param {Function} scheduler - Function to schedule updates
     */
    function setUpdateScheduler(scheduler) {
        requestUpdate = scheduler;
    }

    return {
        useState,
        setCurrentFiber,
        setUpdateScheduler
    };
})();