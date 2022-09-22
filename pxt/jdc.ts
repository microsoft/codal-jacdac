namespace userconfig {
    export const SYSTEM_HEAP_BYTES = 10000
}

namespace jdc {
    /**
     * Set Jacdac parameters.
     */
    //% shim=jdc::setParameters
    export function setParameters(
        dev_class: number,
        fw_version: string,
        dev_name: string
    ) {
        const info = {
            dev_class,
            dev_name,
            fw_version,
        }
        control.simmessages.send(
            "jacdacSettings",
            Buffer.fromUTF8(JSON.stringify(info))
        )
        return
    }

    /**
     * Start jacdac-c stack
     */
    //% shim=jdc::start
    export function start(): void {
        return // do nothin' in sim
    }

    /**
     * Deploy a Jacdac-VM (Jacscript) program.
     */
    //% shim=jdc::deploy
    export function deploy(jacsprog: Buffer): number {
        control.simmessages.send("jacscript", jacsprog)
        // report errors via events?
        return 0
    }

    /**
     * Return number of service instances of given class currently on the bus.
     */
    //% shim=jdc::numServiceInstances
    export function numServiceInstances(serviceClass: number): number {
        return 0
    }
}
