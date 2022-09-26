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

    interface JacdacState {
        selfId: string
        devices: {
            deviceId: string
            services: { name: string;
                serviceClass: number }[]
        }[]
    }

    let _jacdacStateBuffer: Buffer
    function jacdacState(): JacdacState {
        if (!_jacdacStateBuffer) return { selfId: "", devices: [] } as JacdacState
        const state = JSON.parse(_jacdacStateBuffer.toString()) as JacdacState
        return state
    }

    /**
     * Start jacdac-c stack
     */
    //% shim=jdc::start
    export function start(): void {
        control.simmessages.onReceived("jacdacState", (buf: Buffer) => {
            _jacdacStateBuffer = buf
        })
    }

    /**
     * Deploy a Jacdac-VM (Jacscript) program.
     */
    //% shim=jdc::deploy
    export function deploy(jacsprog: Buffer): number {
        control.simmessages.send("jacscript", jacsprog)
        return 0
    }

    /**
     * Return number of service instances of given class currently on the bus.
     */
    //% shim=jdc::numServiceInstances
    export function numServiceInstances(serviceClass: number): number {
        const state = jacdacState()
        if (!state) return 0

        const devices = jacdacState().devices
        let count = 0
        for (let d = 0; d < devices.length; d++) {
            const dev = devices[d]
            for (let s = 0; s < dev.services.length; s++) {
                const srv = dev.services[s]
                if (srv.serviceClass === serviceClass) count++
            }
        }
        return count
    }
}
