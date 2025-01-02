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

    /**
     * Compares dependency arrays to determine if they've changed
     * @param {Array} prevDeps - Previous dependencies array
     * @param {Array} nextDeps - New dependencies array
     * @returns {boolean} True if dependencies have changed
     */
    function depsChanged(prevDeps, nextDeps) {
        if (!prevDeps || !nextDeps || prevDeps.length !== nextDeps.length) {
            return true;
        }
        return prevDeps.some((dep, i) => dep !== nextDeps[i]);
    }

    /**
     * Executes effects for a fiber and its entire tree
     * Handles cleanup of previous effects and running new effects
     * @param {Fiber} fiber - Current fiber being processed
     */
    function runEffects(fiber) {
        if (!fiber) return;

        if (fiber.hooks) {
            fiber.hooks.forEach(hook => {
                if (hook.type === 'effect' && hook.shouldRun) {
                    if (hook.cleanup) {
                        try {
                            hook.cleanup();
                        } catch (error) {
                            console.error('Error in effect cleanup:', error);
                        }
                    }

                    try {
                        hook.cleanup = hook.effect();
                        hook.shouldRun = false;
                    } catch (error) {
                        console.error('Error in effect:', error);
                    }
                }
            });
        }

        if (fiber.child) {
            runEffects(fiber.child);
        }
        if (fiber.sibling) {
            runEffects(fiber.sibling);
        }
    }

    /**
     * Cleans up effects for a fiber's hooks
     * Called during unmount or before re-running effects
     * @param {Fiber} fiber - Fiber whose effects need cleanup
     */
    function cleanupEffects(fiber) {
        if (!fiber.hooks) return;

        fiber.hooks.forEach(hook => {
            if (hook.type === 'effect' && hook.cleanup) {
                try {
                    hook.cleanup();
                } catch (error) {
                    console.error('Error in effect cleanup:', error);
                }
            }
        });
    }

    /**
     * Creates an effect hook for handling side effects
     * @param {Function} callback - Effect function to execute
     * @param {Array} dependencies - Array of dependencies to track for changes
     */
    function useEffect(callback, dependencies) {
        if (!currentFiber) {
            throw new Error('Hooks can only be called inside function components');
        }

        const oldHook = currentFiber.alternate?.hooks?.[hookIndex];

        const hook = {
            type: 'effect',
            effect: callback,
            deps: dependencies,
            cleanup: oldHook ? oldHook.cleanup : undefined
        };

        if (!oldHook || depsChanged(oldHook.deps, dependencies)) {
            hook.shouldRun = true;
        }

        if (!currentFiber.hooks) {
            currentFiber.hooks = [];
        }

        currentFiber.hooks.push(hook);
        hookIndex++;
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
        useEffect,
        runEffects,
        cleanupEffects,
        setCurrentFiber,
        setUpdateScheduler
    };
})();